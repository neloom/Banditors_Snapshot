// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "Components/TextBlock.h"
#include "TRShardWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UTRShardWidget : public UTRWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Update(class AGameCharacter* Target);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ShardCountText;
};
