// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnPoint.generated.h"

UENUM(BlueprintType)
enum class ESpawnPointType : uint8
{
	SPT_ENEMY_REGULAR UMETA(DisplayName = "Enemy monsters (non-boss)"),
	SPT_ENEMY_BOSS UMETA(DisplayName = "Enemy monsters (boss)"),
	SPT_ITEM_DOORKEY UMETA(DisplayName = "Door key (item)"),
};

UCLASS()
class PROJECTTR_API ASpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
	ASpawnPoint();

	// 이 스폰포인트가 가리키는 영역 중 임의의 한 점을 선택해 반환한다
	// NOTE: 기준점은 이 액터의 위치가 된다
	FVector GetRandomSpawnLocation() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// 스폰이 가능한 반경
	UPROPERTY(EditAnywhere)
	float Radius = 10.0f;

	// 네브메쉬 위로만 스폰 영역을 한정할 지 여부
	UPROPERTY(EditAnywhere)
	bool bUseNavMesh = true;

	// 이 스폰포인트가 지정하는 타입
	UPROPERTY(EditAnywhere)
	ESpawnPointType SpawnType = ESpawnPointType::SPT_ENEMY_REGULAR;
};
