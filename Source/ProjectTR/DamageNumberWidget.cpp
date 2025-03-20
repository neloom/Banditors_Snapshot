// Copyright (C) 2024 by Haguk Kim


#include "DamageNumberWidget.h"
#include "DamageNumber.h"
#include "TRMacros.h"
#include "Animation/WidgetAnimation.h"

void UDamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DmgNumberAnim)
	{
		// 필요 시 바인딩 할 것
		/*DmgNumberAnimEndEvent.Clear();
		DmgNumberAnimEndEvent.BindUFunction(this, FName(TEXT("Local_OnDamageNumberAnimEnd")));
		BindToAnimationFinished(DmgNumberAnim, DmgNumberAnimEndEvent);*/

		// 생성과 동시에 재생 시작
		Local_StartDisplaying();
	}
}

void UDamageNumberWidget::Local_StartDisplaying()
{
	PlayAnimation(DmgNumberAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
}

void UDamageNumberWidget::SetDamage(int32 Damage)
{
	NumberText->SetText(FText::FromString(FString::FromInt(Damage)));
}

void UDamageNumberWidget::ClearWidgetData()
{
	NumberText->SetText(FText::FromString(""));
}
