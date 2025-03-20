// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "BaseItem.h"
#include "InvObject.h"
#include "InventoryComponent.h"
#include "TRStructs.generated.h"

USTRUCT(Atomic, BlueprintType)
struct FGameCharacterInstanceData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int8 MeshColorType = 0; // TODO
	UPROPERTY()
	bool bMeshColorTypeCached = false;

	/* AGameCharcater */
	// Health
	UPROPERTY()
	int32 PureMaxHealth = 0; // NOTE: 여기서 캐시되는 값은 아이템 등의 영향을 받지 않은 원래 최대 체력 값을 나타낸다
	UPROPERTY()
	bool bPureMaxHealthCached = false;

	UPROPERTY()
	int32 HealthPoint = 0;
	UPROPERTY()
	bool bHealthPointCached = false;

	// Life
	UPROPERTY()
	bool bHasDied = false;
	UPROPERTY()
	bool bHasDiedCached = false;

	// TODO: Melee Damage 및 부위별 멀티플라이어

	// InventoryComponent
	UPROPERTY()
	TArray<class UInvObject*> InvObjectValues;
	UPROPERTY()
	TArray<FInvObjData> InvObjectDatas;
	UPROPERTY()
	bool bInventoryCached = false; // 두 값을 묶어 관리

	// EquipSystem
	UPROPERTY()
	TArray<class UInvObject*> EquObjectValues;
	UPROPERTY()
	TArray<FInvObjData> EquObjectDatas;
	UPROPERTY()
	int32 CurrentSlot = 0;
	UPROPERTY()
	bool bEquipSysCached = false; // 두 값을 묶어 관리

	/* AFPSCharcter */
	// ExpComponent
	UPROPERTY()
	int32 Experience = 0;
	UPROPERTY()
	bool bExperienceCached = false;

	UPROPERTY()
	int32 LevelExperience = 0;
	UPROPERTY()
	bool bLevelExperienceCached = false;

	UPROPERTY()
	int32 Level = 0;
	UPROPERTY()
	bool bLevelCached = false;

	FGameCharacterInstanceData()
		: MeshColorType(0)
	{}
};

USTRUCT(BlueprintType)
struct FDropItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ABaseItem> ItemRef = nullptr;

	// NOTE: 드랍 확률 혹은 상점 판매 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DropRate = 0.0f;
};

USTRUCT(BlueprintType)
struct FUnfilteredDropItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDropItem DropItem;

	// 드랍이 가능한 최소 층수 (inclusive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinDepth = 0;

	// 드랍이 가능한 최대 층수 (inclusive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxDepth = 0;

	// 후보로 선택될 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SelectWeight = 0.0f;
};

USTRUCT(BlueprintType)
struct FSpawnableMonsterData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ABotCharacter> BotClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Difficulty = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBoss = false;
};

USTRUCT(BlueprintType)
struct FGunPartTierData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReceiverTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BarrelTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GripTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MuzzleTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SightTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StockTier = 1;
};