// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "NullInputModifier.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UNullInputModifier : public UInputModifier
{
	GENERATED_BODY()
	
public:
	// 커스텀 인풋 로직
	virtual struct FInputActionValue ModifyRaw_Implementation(const class UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

protected:
	// 이 InputModifier를 사용하는 인풋과 NullInputKey를 동시에 입력중인 경우 값을 상쇄시켜 0으로 설정한다
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Null Input Key")
	FKey NullInputKey = EKeys::Backslash;
};
