// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "GameCharacter.h"
#include "HUDWidget.generated.h"

/**
 * Deprecated; UTRHUDWidget을 사용할 것
 */
UCLASS()
class PROJECTTR_API UHUDWidget : public UTRWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	AGameCharacter* HUDTarget = nullptr;
};
