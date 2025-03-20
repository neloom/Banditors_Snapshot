// Copyright (C) 2024 by Haguk Kim


#include "ExpComponent.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "TRHUDWidget.h"
#include "TRPlayerController.h"
#include "Math/UnrealMathUtility.h"

UExpComponent::UExpComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UExpComponent::BeginPlay()
{
	Super::BeginPlay();
}

const int UExpComponent::GetTotalExpReqToLvlup(int32 CurrLevel) const
{
	if (CurrLevel < ExpConst::EXP_LVL_MIN)
	{
		UE_LOG(LogTemp, Warning, TEXT("Level is %d, which is below the expected minimum level."), CurrLevel);
	}

	// y = a + K^x 꼴에서 특정 지점 이후부터 y = a + Kx꼴로 필요치가 증가
	if (CurrLevel < ExpConst::EXP_MARKPOINT_LVL)
	{
		return ExpConst::EXP_MIN_REQ + FMath::Pow(ExpConst::EXP_REQ_POWER, CurrLevel);
	}
	return ExpConst::EXP_MIN_REQ + (ExpConst::EXP_REQ_MULTIPLIER * CurrLevel);
}

const int UExpComponent::GetCurrExpReqToLvlup() const
{
	return GetTotalExpReqToLvlup(this->Level) - LevelExperience;
}

void UExpComponent::SetLevel(int Value)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	Level = Value;

	// 서버의 경우 수동 호출
	Local_OnLevelUpdated();
}

void UExpComponent::SetCurrTotalExp(int Value)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	Experience = Value;

	// 서버의 경우 수동 호출
	Local_OnExperienceUpdated();
}

void UExpComponent::SetCurrLevelExp(int Value)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	LevelExperience = Value;

	// 서버의 경우 수동 호출
	Local_OnLvlExperienceUpdated();
}

void UExpComponent::SetCurrShard(int Value)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	Shard = Value;

	// 서버의 경우 수동 호출
	Local_OnShardUpdated();
}

void UExpComponent::GainExp(int32 Exp, float Multiplier)
{
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to change experience / level. Aborting!"));
		return;
	}

	if (Exp <= 0) return;
	int GainedExp = Exp * Multiplier;
	SetCurrLevelExp(LevelExperience + GainedExp);
	SetCurrTotalExp(Experience + GainedExp); // 최신화된 값들로 UI를 업데이트 하려면 SetCurrLevelExp 이후 호출되어야 함
	GainShard(GainedExp);

	while (Level < ExpConst::EXP_LVL_MAX)
	{
		int ExpLeftToLvlup = GetCurrExpReqToLvlup();
		if (ExpLeftToLvlup <= 0)
		{
			// 레벨업
			LevelUp();
			return GainExp(-1 * ExpLeftToLvlup, 1.0f); // 잔여량을 오버플로시킬 때 경험치 배수는 유지되지 않음
		}
		break;
	}
}

void UExpComponent::GainShard(int32 Value)
{
	if (Value <= 0) return;
	SetCurrShard(Shard + Value);
}

void UExpComponent::SpendShard(int32 Value)
{
	if (Value <= 0) return;
	if (Shard < Value)
	{
		SetCurrShard(0);
		UE_LOG(LogTemp, Error, TEXT("SpendShard - Not enough shard. This should've been prevented."));
		return;
	}
	SetCurrShard(Shard - Value);
}

void UExpComponent::LoseLevelExp()
{
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to change experience / level. Aborting!"));
		return;
	}
	SetCurrLevelExp(0);
}

void UExpComponent::ResetAllExp()
{
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to change experience / level. Aborting!"));
		return;
	}
	SetLevel(ExpConst::EXP_LVL_MIN);
	SetCurrLevelExp(0);
	SetCurrTotalExp(0); // 최신화된 값들로 UI를 업데이트 하려면 SetCurrLevelExp 이후 호출되어야 함
}

void UExpComponent::LevelUp()
{
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to change experience / level. Aborting!"));
		return;
	}
	SetLevel(Level + 1);
	SetCurrLevelExp(0);

	// 캐릭터의 레벨업 시 로직 수행
	AFPSCharacter* FPSOwner = Cast<AFPSCharacter>(GetOwner());
	if (FPSOwner)
	{
		FPSOwner->Server_OnLevelUp(Level);
	}

	/////TESTING
	FString DebugString = FString::Printf(TEXT("Level up! CurrLevel: %d Total exp: %d Exp required for next lvl: %d"), GetLevel(), GetCurrTotalExp(), GetCurrExpReqToLvlup());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, DebugString);
}

void UExpComponent::OnRep_Experience()
{
	Local_OnExperienceUpdated();
}

void UExpComponent::Local_OnExperienceUpdated()
{
	// UI 업데이트 (서버,클라)
	AGameCharacter* TROwner = Cast<AGameCharacter>(GetOwner());
	if (TROwner && TROwner->Local_GetBoundHUDWidget().IsValid())
	{
		TROwner->Local_GetBoundHUDWidget()->UpdateExp();
	}
}

void UExpComponent::OnRep_LevelExperience()
{
	Local_OnLvlExperienceUpdated();
}

void UExpComponent::Local_OnLvlExperienceUpdated()
{
	// NOTE: UI의 경우 Local_OnExperienceUpdated에서 처리 
}

void UExpComponent::OnRep_Level()
{
	Local_OnLevelUpdated();
}

void UExpComponent::Local_OnLevelUpdated()
{
	// NOTE: UI의 경우 Local_OnExperienceUpdated에서 처리 
}

void UExpComponent::OnRep_Shard()
{
	Local_OnShardUpdated();
}

void UExpComponent::Local_OnShardUpdated()
{
	// UI 업데이트 (서버,클라)
	AGameCharacter* TROwner = Cast<AGameCharacter>(GetOwner());
	if (TROwner && TROwner->Local_GetBoundHUDWidget().IsValid())
	{
		TROwner->Local_GetBoundHUDWidget()->UpdateShards();
	}
}
