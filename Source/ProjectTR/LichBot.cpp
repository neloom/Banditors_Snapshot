// Copyright (C) 2024 by Haguk Kim


#include "LichBot.h"

ALichBot::ALichBot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// 소켓 네임 수정
	HeadSockName = "Head_M";
	TorsoSockName = "Spine2_M";
	PelvisSockName = "Pelvis";
	RArmUpperSockName = "Shoulder_R";
	RArmLowerSockName = "Elbow_R";
	RHandSockName = "Wrist_R";
	LArmUpperSockName = "Shoulder_L";
	LArmLowerSockName = "Elbow_L";
	LHandSockName = "Wrist_L";
	RLegUpperSockName = "Tail21_R";
	RLegLowerSockName = "Tail23_R";
	RFootSockName = "Tail25_R";
	LLegUpperSockName = "Tail8_L";
	LLegLowerSockName = "Tail10_L";
	LFootSockName = "Tail12_L";

	// 소켓 정보를 수정했으므로 히트박스 재부착
	InitHitbox();
}