// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "Components/TextBlock.h"
#include "DamageNumberWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UDamageNumberWidget : public UTRWidget
{
	GENERATED_BODY()
	

public:
	virtual void NativeConstruct() override;

	void Local_StartDisplaying();
	void SetDamage(int32 Damage);
	void ClearWidgetData();

	// 필요 시 선언
	/*UFUNCTION(BlueprintCallable)
	void Local_OnDamageNumberAnimEnd();*/

public:
	// BP에서 정의된 구성요소와 바인딩되어있다
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NumberText = nullptr;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* DmgNumberAnim = nullptr;

	// 데미지 애니메이션 종료 시 콜백
	FWidgetAnimationDynamicEvent DmgNumberAnimEndEvent;
};
