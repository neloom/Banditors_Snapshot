// Copyright (C) 2024 by Haguk Kim


#include "StatusEffect.h"
#include "GameCharacter.h"
#include "Kismet/GameplayStatics.h"

void UStatusEffect::ChangeRootSetRecursive(bool bAddRoot, UObject* NewOuter)
{
	if (bAddRoot) this->AddToRoot();
	else this->RemoveFromRoot();

	if (NewOuter) Rename(nullptr, NewOuter);
	// 필요 시 오버라이드
}

void UStatusEffect::SetStatusEffectId(const FString& Id)
{
	if (!Server_StatusEffectId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UStatusEffect::SetStatId - ID is not empty(%s)! Please check if this is intended."), *Server_StatusEffectId);
	}
	Server_StatusEffectId = Id;
}

void UStatusEffect::Server_SetDuration(float Seconds, EStatTimerHandleMethod HandleMethod)
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (!ParentChar.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_SetDuration - Parent character is not registered! It is HIGHLY recommended to cache the owner before setting the duration. Please check."));
	}

	if (World->GetTimerManager().IsTimerActive(Server_DurationTimer))
	{
		float RemainingTime = World->GetTimerManager().GetTimerRemaining(Server_DurationTimer);

		if (HandleMethod == EStatTimerHandleMethod::STHM_UseLargerVal && RemainingTime >= Seconds)
		{
			return;
		}
		else if (HandleMethod == EStatTimerHandleMethod::STHM_UseSmallerVal && RemainingTime <= Seconds)
		{
			return;
		}

		if (Seconds <= 0.0f)
		{
			// 타이머 비활성화; 이때 기존 설정된 콜백이 반드시 호출되어야 하므로 Clear 대신 아주 짧은 시간으로 재설정한다
			World->GetTimerManager().SetTimer(Server_DurationTimer, this, &UStatusEffect::Server_OnDurationEnd, 0.1f, false);
			UE_LOG(LogTemp, Warning, TEXT("Server_SetDuration - Active timer was modified to become inactive. This is generally not recommended. If you are trying to remove the statuseffect, try removing it directly instead of timer modification."));
			return;
		}
	}
	else
	{
		if (Seconds <= 0.0f)
		{
			// 타이머가 이미 비활성화됨
			return;
		}
	}

	// 기존 콜백 오버라이드
	float TimerSec = FMath::Max(0.1f, Seconds);
	World->GetTimerManager().SetTimer(Server_DurationTimer, this, &UStatusEffect::Server_OnDurationEnd, TimerSec, false);
}

void UStatusEffect::Server_AddDuration(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	if (!World) return;

	float RemainingTime = 0.0f;
	if (!World->GetTimerManager().IsTimerActive(Server_DurationTimer))
	{
		RemainingTime = World->GetTimerManager().GetTimerRemaining(Server_DurationTimer);
	}
	Server_SetDuration(RemainingTime + DeltaSeconds, EStatTimerHandleMethod::STHM_ForceOverride);
}

void UStatusEffect::Server_OnSelfAddedToParent(AGameCharacter* Parent, AGameCharacter* Applier)
{
	if (!IsValid(Parent))
	{
		UE_LOG(LogTemp, Error, TEXT("Server_OnSelfAddedToParent - Invalid parent ptr!"));
		return;
	}
	ParentChar = MakeWeakObjectPtr<AGameCharacter>(Parent);
	if (Applier)
	{
		ApplierChar = MakeWeakObjectPtr<AGameCharacter>(Applier);
	}

	// 데미지 연산 활성화
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_OnSelfAddedToParent - Invalid world!"));
		return;
	}
	if (EffectDamage.DamageRateSec < 0.0f)
	{
		// 데미지 처리 X
	}
	else
	{
		FTimerDelegate TimerDel;
		TimerDel.BindUObject(this, &UStatusEffect::Server_ApplyDamageOnce, Parent, Applier);
		if (FMath::IsNearlyZero(EffectDamage.DamageRateSec))
		{
			World->GetTimerManager().SetTimerForNextTick(TimerDel);
		}
		else
		{
			World->GetTimerManager().SetTimer(DamageTimer, TimerDel, EffectDamage.DamageRateSec, true/* 상태이상 자체가 종료되기 전까지 루프 */);
		}
	}
}

void UStatusEffect::Server_ValidateSelf(float InitialStatDuration)
{
	if (InitialStatDuration > 0.0f && InitialStatDuration < EffectDamage.DamageRateSec)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_ValidateSelf - StatusEffect DamageRateSec is longer than its total duration. Damage effect can be ignored."));
	}
	if (Server_StatusEffectId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_ValidateSelf - StatusEffect ID is empty! This is not recommended."));
	}
	if (StatusEffectName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_ValidateSelf - StatusEffect name is empty! This is not recommended."));
	}
}

void UStatusEffect::Server_InvalidateSelf()
{
	if (!ParentChar.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_InvalidateSelf - Invalid parent ptr! Possibility of memory leak!"));
		return;
	}
	ParentChar->Server_RemoveStatEffect(this);
	ParentChar.Get();

	// 데미지 연산 비활성화
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_InvalidateSelf - Invalid world!"));
		return;
	}
	World->GetTimerManager().ClearTimer(DamageTimer);

	// 이후에는 GC에 의해 파괴됨
}

void UStatusEffect::Server_OnDurationEnd()
{
	TR_PRINT_FSTRING("StatusEffect %s duration end", *this->Server_StatusEffectId);
	Server_InvalidateSelf();
}

void UStatusEffect::Server_ApplyDamageOnce(AGameCharacter* Target, AGameCharacter* Applier)
{
	if (!Target)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_ApplyDamageOnce - Invalid arguments!"));
		return;
	}

	APawn* DamageCauser = Applier;
	FHitResult StatusEffDmgHit;
	const FVector& TargetLoc = Target->GetActorLocation();
	StatusEffDmgHit.ImpactNormal = FVector::UpVector;
	StatusEffDmgHit.ImpactPoint = TargetLoc;
	StatusEffDmgHit.TraceStart = TargetLoc;
	StatusEffDmgHit.Location = TargetLoc;
	
	UGameplayStatics::ApplyPointDamage(
		Target, 
		EffectDamage.DamageVal, 
		Target->GetActorForwardVector() * -1, // 타깃 기준 하단에서 데미지를 입은 것처럼 처리 (그래야 히트마커 위치가 직관적임)
		StatusEffDmgHit, 
		DamageCauser != nullptr ? DamageCauser->GetController() : nullptr,
		DamageCauser,
		EffectDamage.DamageType
	);

	// 값 갱신
	EffectDamage.DamageVal += EffectDamage.DamageDeltaPerCalc;
	EffectDamage.DamageVal *= EffectDamage.DamageMultPerCalc;
}
