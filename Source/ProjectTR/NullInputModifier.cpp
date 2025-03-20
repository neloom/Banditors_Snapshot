// Copyright (C) 2024 by Haguk Kim


#include "NullInputModifier.h"
#include "EnhancedPlayerInput.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"

FInputActionValue UNullInputModifier::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	if (NullInputKey.IsValid() && PlayerInput->GetOuterAPlayerController())
	{
		if (PlayerInput->GetOuterAPlayerController()->IsInputKeyDown(NullInputKey))
		{
			// 인풋이 상쇄되도록 0으로 설정한다
			return Super::ModifyRaw_Implementation(PlayerInput, FVector::ZeroVector, DeltaTime);
		}
	}
	return Super::ModifyRaw_Implementation(PlayerInput, CurrentValue, DeltaTime);
}
