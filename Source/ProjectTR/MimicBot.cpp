// Copyright (C) 2024 by Haguk Kim


#include "MimicBot.h"

AMimicBot::AMimicBot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// 미믹의 경우 회전 속도를 높임
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->RotationRate = FRotator(0, 360, 0);
	}

	// 소켓 네임 수정
	HeadSockName = "mimic_top";
	TorsoSockName = "mimic_body";
	PelvisSockName = "mimic_body";
	RArmUpperSockName = "mimic_tongue1";
	RArmLowerSockName = "mimic_tongue4";
	RHandSockName = "mimic_tongue8";
	LArmUpperSockName = "mimic_tongue1";
	LArmLowerSockName = "mimic_tongue4";
	LHandSockName = "mimic_tongue8";
	RLegUpperSockName = "mimic_body";
	RLegLowerSockName = "mimic_body";
	RFootSockName = "mimic_body";
	LLegUpperSockName = "mimic_body";
	LLegLowerSockName = "mimic_body";
	LFootSockName = "mimic_body";

	// 소켓 정보를 수정했으므로 히트박스 재부착
	InitHitbox();
}

void AMimicBot::SetupChestInfo(const TArray<FDropItem>& ChestRewards)
{
	DropRewards = ChestRewards;
}
