// Copyright (C) 2024 by Haguk Kim


#include "GunItemData.h"
#include "GunItem.h"

UGunItemData::UGunItemData()
{
	TestVal = 800.0f;
}

bool UGunItemData::CacheItem(const ABaseItem* Item)
{
	if (!Super::CacheItem(Item)) return false;

	const AGunItem* GunItem = Cast<AGunItem>(Item);
	if (!IsValid(GunItem))
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to cache a non-GunItem %s to a GunItemData."), *(Item->GetName()));
		return false;
	}
	if (!GunItem->GetReceiver())
	{
		UE_LOG(LogTemp, Error, TEXT("GunItem %s has no receiver! Failed to CacheItem."), *(GunItem->GetName()));
		return false;
	}

	// 주의:
	// GetClass() 대신 StaticClass()를 사용할 경우 제대로 동작하지 않음
	if (GunItem->GetReceiver()) CacheReceiverClass(GunItem->GetReceiver()->GetClass());
	if (GunItem->GetBarrel()) CacheBarrelClass(GunItem->GetBarrel()->GetClass());
	if (GunItem->GetGrip()) CacheGripClass(GunItem->GetGrip()->GetClass());
	if (GunItem->GetMagazine()) CacheMagazineClass(GunItem->GetMagazine()->GetClass());
	if (GunItem->GetMuzzle()) CacheMuzzleClass(GunItem->GetMuzzle()->GetClass());
	if (GunItem->GetSight()) CacheSightClass(GunItem->GetSight()->GetClass());
	if (GunItem->GetStock()) CacheStockClass(GunItem->GetStock()->GetClass());

	// 그외 데이터 캐싱
	CacheCurrAmmo(GunItem->CurrAmmo);
	return true;
}

void UGunItemData::ChangeRootSetRecursive(bool bAddRoot, UObject* NewOuter)
{
	Super::ChangeRootSetRecursive(bAddRoot, NewOuter);

	// NOTE: SubclassOf의 아우터 변경 시도 시 크래시 발생하므로 생략
	if (CachedReceiverClass)
	{
		if (bAddRoot) CachedReceiverClass->AddToRoot();
		else CachedReceiverClass->RemoveFromRoot();
	}
	if (CachedBarrelClass)
	{
		if (bAddRoot) CachedBarrelClass->AddToRoot();
		else CachedBarrelClass->RemoveFromRoot();
	}
	if (CachedGripClass)
	{
		if (bAddRoot) CachedGripClass->AddToRoot();
		else CachedGripClass->RemoveFromRoot();
	}
	if (CachedMagazineClass)
	{
		if (bAddRoot) CachedMagazineClass->AddToRoot();
		else CachedMagazineClass->RemoveFromRoot();
	}
	if (CachedMuzzleClass)
	{
		if (bAddRoot) CachedMuzzleClass->AddToRoot();
		else CachedMuzzleClass->RemoveFromRoot();
	}
	if (CachedSightClass)
	{
		if (bAddRoot) CachedSightClass->AddToRoot();
		else CachedSightClass->RemoveFromRoot();
	}
	if (CachedStockClass)
	{
		if (bAddRoot) CachedStockClass->AddToRoot();
		else CachedStockClass->RemoveFromRoot();
	}
}