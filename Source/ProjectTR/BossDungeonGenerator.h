// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRDungeonGenerator.h"
#include "BossDungeonGenerator.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ABossDungeonGenerator : public ATRDungeonGenerator
{
	GENERATED_BODY()

public:
	ABossDungeonGenerator();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void OnBossFightCleared();

	virtual void Server_UpdateGameModeAfterGeneration() const;

protected:
	virtual void ProcessDungeonActorDuringInit(class ADungeonActor* DungeonActor) override;

public:
	// 보스전 클리어 시 트리거되는 던전 액터들의 목록
	UPROPERTY(EditInstanceOnly)
	TArray<TWeakObjectPtr<ADungeonActor>> OnBossFightClearTrigger;
};
