// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "Components/TextBlock.h"
#include "PlayerNameTagWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UPlayerNameTagWidget : public UTRWidget
{
	GENERATED_BODY()
	
public:
	void SetName(FString Name);

public:
	// BP에서 정의된 구성요소와 바인딩되어있다
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameTagTextBox;
};
