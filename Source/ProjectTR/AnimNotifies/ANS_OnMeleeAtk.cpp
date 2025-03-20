// Copyright (C) 2024 by Haguk Kim


#include "ANS_OnMeleeAtk.h"
#include "TRMacros.h"
#include "TRUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameCharacter.h"
#include "BotCharacter.h"

UANS_OnMeleeAtk::UANS_OnMeleeAtk()
{
	//HitObjectTypes.Add()
}

void UANS_OnMeleeAtk::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	// 매번 초기화
	HitTargetsForCurrSequence.Empty();
}

void UANS_OnMeleeAtk::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	if (!MeshComp->GetOwner() || !MeshComp->GetOwner()->HasAuthority()) return;

	FName MeleeSockName;
	if (bIsRightHandMelee) MeleeSockName = TEXT(MELEE_ATK_SOCKET_R);
	else MeleeSockName = TEXT(MELEE_ATK_SOCKET_L);

	FVector SocketLocation = MeshComp->GetSocketLocation(MeleeSockName);
	TArray<AActor*> ActorsToIgnore;
	//ActorsToIgnore.Add(MeshComp->GetOwner()); // bIgnoreSelf = true 설정 시 필요 없음
	FHitResult HitResult;

	bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(MeshComp, SocketLocation, SocketLocation, SocketHitRadius, HitObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);
	if (bHit)
	{
		AGameCharacter* HitTarget = Cast<AGameCharacter>(HitResult.GetActor());
		AGameCharacter* Attacker = Cast<AGameCharacter>(MeshComp->GetOwner());
		if (!HitTarget || !Attacker || TRUtils::IsAllyWith(HitTarget, Attacker)) return; // 아군 무시

		HitTargetsForCurrSequence.Add(HitTarget, HitResult);
	}
}

void UANS_OnMeleeAtk::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp->GetOwner() || !MeshComp->GetOwner()->HasAuthority()) return;

	// 모든 대상들에 대해 밀리 로직 수행
	AGameCharacter* Attacker = Cast<AGameCharacter>(MeshComp->GetOwner());
	if (IsValid(Attacker))
	{
		for (const TPair<AGameCharacter*, FHitResult>& Pair : HitTargetsForCurrSequence)
		{
			Attacker->ProcessMeleeAtk(Pair.Get<0>(), Pair.Get<1>());
		}
	}
}