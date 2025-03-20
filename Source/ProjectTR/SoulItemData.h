// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "TRStructs.h"
#include "SoulItemData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API USoulItemData : public UItemData
{
	GENERATED_BODY()
	
	USoulItemData();

public:
	virtual bool CacheItem(const class ABaseItem* Item) override;

	/* Getters */
	TSubclassOf<class AFPSCharacter> GetCachedCharacterClass() { return Server_CharacterClass; }
	class ATRPlayerController* GetCachedController() { return Server_Controller; }
	const struct FGameCharacterInstanceData& GetCachedInstanceData() { return Server_InstanceData; }

	/* Setters */
	void CacheCharacterClass(TSubclassOf<class AFPSCharacter> Class) { Server_CharacterClass = Class; }
	void CacheController(class ATRPlayerController* Controller) { Server_Controller = Controller; }
	void CacheInstanceData(const struct FGameCharacterInstanceData Data) { Server_InstanceData = Data; }

protected:
	// 부활에 쓰이는 캐릭터 클래스
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class AFPSCharacter> Server_CharacterClass = nullptr;

	// 부활 대상 플레이어
	UPROPERTY(BlueprintReadOnly)
	class ATRPlayerController* Server_Controller = nullptr;

	// 부활 시 사용할 복구 데이터
	UPROPERTY(BlueprintReadOnly)
	struct FGameCharacterInstanceData Server_InstanceData;
};
