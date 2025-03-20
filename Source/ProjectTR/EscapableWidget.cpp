// Copyright (C) 2024 by Haguk Kim


#include "EscapableWidget.h"
#include "TRPlayerController.h"
#include "TRMacros.h"

void UEscapableWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
}

FReply UEscapableWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    FKey PressedKey = InKeyEvent.GetKey();
    if (EscapeKeys.Contains(PressedKey))
    {
        ATRPlayerController* OwningPC = GetOwningPlayer<ATRPlayerController>();
        if (OwningPC && OwningPC->IsLocalController())
        {
            OwningPC->Local_DerefWidget(this);
            OwningPC->Local_FocusGame(false, true);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UEscapableWidget::NativeOnKeyDown - Player controller is invalid! Processing emergency widget dereferencing."));
            RemoveFromParent();
            RemoveFromRoot();
        }
        return FReply::Handled();
    }
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
