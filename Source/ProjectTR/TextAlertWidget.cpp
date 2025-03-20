// Copyright (C) 2024 by Haguk Kim


#include "TextAlertWidget.h"
#include "TimerManager.h"
#include "TRPlayerController.h"

UTextAlertWidget::UTextAlertWidget()
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UTextAlertWidget::SetTextForDuration(const FString& Text, float Duration)
{
	TextBox->SetText(FText::FromString(Text));
	CurrentWaitDuration = Duration;

	// 팝업 애니메이션의 종료 시점을 기준으로 Duration만큼 대기한다
	if (TextPopUpAnim)
	{
		TextPopUpAnimEndEvent.Clear();
		TextPopUpAnimEndEvent.BindUFunction(this, FName(TEXT("WaitForDuration")));
		BindToAnimationFinished(TextPopUpAnim, TextPopUpAnimEndEvent);

		PlayAnimation(TextPopUpAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
	
}

void UTextAlertWidget::DestroySelf()
{
	ATRPlayerController* PC = GetOwningPlayer<ATRPlayerController>();
	if (PC)
	{
		PC->Local_DerefWidget(this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UTextAlertWidget::DestroySelf - Unable to get player controller! Initiating emergency destruction procedure."));
		RemoveFromParent();
		RemoveFromRoot();
	}
}

void UTextAlertWidget::WaitForDuration()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UTextAlertWidget::DestroySelf, CurrentWaitDuration, false);
}
