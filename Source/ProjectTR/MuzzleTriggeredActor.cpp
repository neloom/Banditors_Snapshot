// Copyright (C) 2024 by Haguk Kim


#include "MuzzleTriggeredActor.h"
#include "GameCharacter.h"
#include "Components/BoxComponent.h"

AMuzzleTriggeredActor::AMuzzleTriggeredActor()
{
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MuzzleRayDetectBox"));
	check(TriggerBox);
	TriggerBox->SetSimulatePhysics(false);
	TriggerBox->SetEnableGravity(false);
	TriggerBox->SetCollisionProfileName(TEXT("MuzzleTriggeredActor"));
	TriggerBox->SetGenerateOverlapEvents(false); // 불필요
	TriggerBox->SetShouldUpdatePhysicsVolume(false);
	TriggerBox->SetupAttachment(RootComponent);
}

void AMuzzleTriggeredActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMuzzleTriggeredActor::MuzzleTrigger(AGameCharacter* TriggeredBy)
{
	OnMuzzleTriggered(TriggeredBy);
}

void AMuzzleTriggeredActor::OnMuzzleTriggered(AGameCharacter* TriggeredBy)
{
	// 세부 로직은 상속받은 액터에서 구현한다
}

