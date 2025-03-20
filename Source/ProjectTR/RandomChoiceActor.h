// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "DungeonActor.h"
#include "TRUtils.h"
#include "RandomChoiceActor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ARandomChoiceActor : public ADungeonActor
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void Initialize() override;

public:
	ARandomChoiceActor();

	// 이 액터의 소환 후보들 중 하나를 선택해 그 액터를 소환하고, 이 액터를 파괴한다
	// 만약 소환 후보가 없을 경우 아무 것도 처리하지 않고 파괴한다
	// 소환에 성공한 경우 소환 대상의 포인터를 반환한다
	// bApplyGravity가 true일 경우 이 액터의 위치에서 아래로 Line trace를 해 첫 충돌 지점에 소환한다
	UFUNCTION()
	ADungeonActor* Server_SelectAndReplaceWithRandomCand(bool bApplyGravity);

	// 주어진 위치에서 바닥으로 Linetrace를 시도해 첫 충돌 지점을 반환한다
	// 충돌이 없을 경우 이 액터의 위치를 반환한다
	FVector GetFootLocation(FVector RootLocation, float MaxLineLength = 10000.0f);
	
protected:
	// 소환 후보
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<ADungeonActor>> SpawnCandidates;

	// 각 후보의 소환 확률(weight)
	UPROPERTY(EditAnywhere)
	TArray<float> SpawnChances;

	// 무언가 하나라도 소환될 확률 [0, 1]
	// 이 값이 0일 경우 후보나 weight 값의 존재여부와 무관하게 아무것도 생성되지 않는다
	UPROPERTY(EditAnywhere)
	float GenerationChance = 1.0f;
};
