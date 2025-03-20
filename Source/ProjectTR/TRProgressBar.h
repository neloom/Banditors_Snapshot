// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "TRProgressBar.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UTRProgressBar : public UTRWidget
{
	GENERATED_BODY()
	
public:
	void SetValue(float Percentage);

	// 블루프린트에서 구현
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Custom")
	void SetPercentage(float Percentage);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* Back;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* Front;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	USlider* Slider;
};
