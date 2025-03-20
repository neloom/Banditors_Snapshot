// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "MuzzleTriggeredActor.h"
#include "InteractableShop.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API AInteractableShop : public AMuzzleTriggeredActor
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void OnMuzzleTriggered(class AGameCharacter* TriggeredBy) override;

protected:
	// 실제 로직 처리 액터에 대한 참조
	TObjectPtr<class ATRShop> ShopLogicActorRef = nullptr;

	// 최초 바인딩할 상점 로직 액터
	// 상점의 종류를 결정하는 등에 사용할 수 있다
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ATRShop> ShopActorClass = nullptr;
};
