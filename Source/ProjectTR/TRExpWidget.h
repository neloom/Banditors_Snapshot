// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "TRExpWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UTRExpWidget : public UTRWidget
{
	GENERATED_BODY()

public:
	void Update(class AGameCharacter* Target);
	
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CurrLevelText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* ExpLeftUntilLvlUpBar = nullptr;
};
