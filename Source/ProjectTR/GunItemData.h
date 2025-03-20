// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "GunItemData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGunItemData : public UItemData
{
	GENERATED_BODY()

	UGunItemData();
	
public:
	virtual bool CacheItem(const class ABaseItem* Item) override;

	virtual void ChangeRootSetRecursive(bool bAddRoot, UObject* NewOuter = nullptr) override;

	/* Getters */
	TSubclassOf<class UGunPartComponent> GetCachedBarrelClass() { return CachedBarrelClass; }
	TSubclassOf<class UGunPartComponent> GetCachedGripClass() { return CachedGripClass; }
	TSubclassOf<class UGunPartComponent> GetCachedMagazineClass() { return CachedMagazineClass; }
	TSubclassOf<class UGunPartComponent> GetCachedMuzzleClass() { return CachedMuzzleClass; }
	TSubclassOf<class UGunPartComponent> GetCachedReceiverClass() { return CachedReceiverClass; }
	TSubclassOf<class UGunPartComponent> GetCachedSightClass() { return CachedSightClass; }
	TSubclassOf<class UGunPartComponent> GetCachedStockClass() { return CachedStockClass; }

	int32 GetCachedCurrAmmo() { return CachedCurrAmmo; }

	/* Setters */
	void CacheBarrelClass(TSubclassOf<class UGunPartComponent> Class) { CachedBarrelClass = Class; }
	void CacheGripClass(TSubclassOf<class UGunPartComponent> Class) { CachedGripClass = Class; }
	void CacheMagazineClass(TSubclassOf<class UGunPartComponent> Class) { CachedMagazineClass = Class; }
	void CacheMuzzleClass(TSubclassOf<class UGunPartComponent> Class) { CachedMuzzleClass = Class; }
	void CacheReceiverClass(TSubclassOf<class UGunPartComponent> Class) { CachedReceiverClass = Class; }
	void CacheSightClass(TSubclassOf<class UGunPartComponent> Class) { CachedSightClass = Class; }
	void CacheStockClass(TSubclassOf<class UGunPartComponent> Class) { CachedStockClass = Class; }

	void CacheCurrAmmo(int32 Value) { CachedCurrAmmo = Value; }

protected:
	// 총기 파츠 컴포넌트 클래스 캐싱
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class UGunPartComponent> CachedReceiverClass = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class UGunPartComponent> CachedBarrelClass = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class UGunPartComponent> CachedGripClass = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class UGunPartComponent> CachedMagazineClass = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class UGunPartComponent> CachedMuzzleClass = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class UGunPartComponent> CachedSightClass = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class UGunPartComponent> CachedStockClass = nullptr;

	// NOTE: 잔탄이 최대치만큼 차야 하지만 정확한 최대 탄약을 모르는 경우 -1을 저장한다
	// 이 경우 데이터 복구 시점에서의 최대 탄약만큼 탄약이 채워진다
	// BaseItem 없이 데이터를 변경해야 할 경우 (인벤토리에 있는 총의 탄약을 채우는 경우 등) 사용할 수 있다
	UPROPERTY(BlueprintReadOnly)
	int32 CachedCurrAmmo = 0;
};
