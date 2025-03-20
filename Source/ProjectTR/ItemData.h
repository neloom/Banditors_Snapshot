// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedObject.h"
#include "ItemData.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTTR_API UItemData : public UReplicatedObject
{
	GENERATED_BODY()

public:
	UItemData();

	// 아이템의 정보를 이 데이터에 캐싱한다
	// 성공 여부를 반환한다
	virtual bool CacheItem(const class ABaseItem* Item);

	// 이 오브젝트 및 이 오브젝트 하위의 모든 오브젝트들에 대해 수동으로 루트셋에 추가 / 삭제한다
	// NewOuter가 nullptr가 아닌 경우 아우터도 수동으로 변경한다
	virtual void ChangeRootSetRecursive(bool bAddRoot, UObject* NewOuter = nullptr);

	// 아이콘 초기화 여부를 캐싱한다
	// 1회 이상 동적 생성했을 경우 true
	void CacheHasInitIcon(bool bValue) { bCachedHasInitIcon = bValue; }
	bool GetCachedHasInitIcon() { return bCachedHasInitIcon; }

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bCachedHasInitIcon = false;

#pragma region /** Debug */
public:
	UPROPERTY(EditAnywhere)
	float TestVal = 0.0f;
#pragma endregion
};
