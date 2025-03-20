// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "PickupItem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API APickupItem : public ABaseItem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Use")
	class UUseItemComponent* UseComponent = nullptr;

protected:
	// 이 아이템의 소유자
	TWeakPtr<class AGameCharacter> Owner = nullptr;
};
