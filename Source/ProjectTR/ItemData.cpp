// Copyright (C) 2024 by Haguk Kim


#include "ItemData.h"
#include "BaseItem.h"

UItemData::UItemData()
{
}

bool UItemData::CacheItem(const ABaseItem* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Error, TEXT("CacheItem - Item is null!"));
		return false;
	}
	if (!Item->HasAuthority()) return false;

	CacheHasInitIcon(Item->bServer_HasInitializedIcon);
	return true; // 필요 시 오버라이드 후 로직 작성
}

void UItemData::ChangeRootSetRecursive(bool bAddRoot, UObject* NewOuter)
{
	if (bAddRoot) this->AddToRoot();
	else this->RemoveFromRoot();
	
	if (NewOuter) Rename(nullptr, NewOuter);
	// 필요 시 오버라이드
}