// Copyright (C) 2025 by Haguk Kim


#include "TRHealthBar.h"
#include "GameCharacter.h"

void UTRHealthBar::Update(AGameCharacter* Target)
{
	if (!CurrHealthText || !MaxHealthText || !HealthBar) return;

	if (!IsValid(Target))
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		SetVisibility(ESlateVisibility::Visible);
		int32 CurrHealth = Target->GetStat_CurrentHealth();
		int32 MaxHealth = Target->GetStat_MaxHealth();
		if (Target->GetHasDied())
		{
			CurrHealth = 0;
		}
		CurrHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrHealth)));
		MaxHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d"), MaxHealth)));

		float HpPercent = 0.0f;
		if (MaxHealth > 0)
		{
			HpPercent = (float) CurrHealth / MaxHealth;
		}
		HealthBar->SetPercent(HpPercent);
	}
}
