// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "EscapableWidget.h"
#include "TRShop.h"
#include "ShopBasedWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UShopBasedWidget : public UEscapableWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	ATRShop* ShopActor = nullptr;
};
