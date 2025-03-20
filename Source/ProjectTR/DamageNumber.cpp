// Copyright (C) 2024 by Haguk Kim


#include "DamageNumber.h"
#include "Blueprint/UserWidget.h"
#include "TRMacros.h"
#include "Animation/WidgetAnimation.h"
#include "Components/WidgetComponent.h"

ADamageNumber::ADamageNumber()
{
}

void ADamageNumber::BeginPlay()
{
	Super::BeginPlay();

	if (DamageNumberWidgetClass)
	{
		UDamageNumberWidget* Widget = CreateWidget<UDamageNumberWidget>(GetWorld(), DamageNumberWidgetClass);
		if (Widget)
		{
			Widget->SetDamage(DamageValue);
			// TODO: 다른 정보 필요 시 추가

			UWidgetComponent* WidgetComp = GetWidgetComp();
			check(WidgetComp != nullptr);
			WidgetComp->SetWidget(Widget);

			if (HasAuthority() && Widget->DmgNumberAnim)
			{
				// Deferred lifespan
				// 위젯 애니메이션보다 약간 더 긴 수명을 주어 네트워크 딜레이를 보완한다
				SetLifeSpan(Widget->DmgNumberAnim->GetEndTime() + 3.0f);
			}
		}
	}
}

void ADamageNumber::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	TR_PRINT("ADamageNumber endplay");
}

void ADamageNumber::ClearData()
{
	Super::ClearData();
	DamageValue = 0;
}

