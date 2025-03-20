// Copyright (C) 2024 by Haguk Kim


#include "BossDungeonGenerator.h"
#include "ProjectTRGameModeBase.h"
#include "RoomData.h"

ABossDungeonGenerator::ABossDungeonGenerator()
{

}

void ABossDungeonGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void ABossDungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABossDungeonGenerator::OnBossFightCleared()
{
	for (TWeakObjectPtr<ADungeonActor> DungeonActor : OnBossFightClearTrigger)
	{
		if (DungeonActor.IsValid())
		{
			DungeonActor->TriggerThis();
		}
	}
}

void ABossDungeonGenerator::Server_UpdateGameModeAfterGeneration() const
{
	if (!ExitingRoomInst)
	{
		UE_LOG(LogTemp, Error, TEXT("ABossDungeonGenerator::Server_UpdateGameModeDuringGeneration - Invalid exit room(=boss room) instance! Bossfight will fail!"));
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ABossDungeonGenerator::Server_UpdateGameModeDuringGeneration - Invalid world!"));
		return;
	}
	AProjectTRGameModeBase* TRGM = World->GetAuthGameMode<AProjectTRGameModeBase>();
	if (!TRGM)
	{
		UE_LOG(LogTemp, Error, TEXT("ABossDungeonGenerator::Server_UpdateGameModeDuringGeneration - Invalid game mode!"));
		return;
	}
	TRGM->BossRoom = ExitingRoomInst;
	return Super::Server_UpdateGameModeAfterGeneration();
}

void ABossDungeonGenerator::ProcessDungeonActorDuringInit(ADungeonActor* DungeonActor)
{
	if (DungeonActor->bTriggerOnBossFightClear)
	{
		OnBossFightClearTrigger.Add(MakeWeakObjectPtr<ADungeonActor>(DungeonActor));
	}
}
