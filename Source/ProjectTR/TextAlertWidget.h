// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "Components/TextBlock.h"
#include "TextAlertWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UTextAlertWidget : public UTRWidget
{
	GENERATED_BODY()
	
public:
	UTextAlertWidget();

	// 주어진 시간동안 텍스트를 변경하고, 이후 리셋한다
	void SetTextForDuration(const FString& Text, float Duration);

	// 스스로를 제거한다
	void DestroySelf();

	// 텍스트를 일정 시간동안 보여준다; 즉 일정 시간 동안 아무 것도 처리하지 않는다
	UFUNCTION()
	void WaitForDuration();

public:
	// 타이머 핸들
	FTimerHandle TimerHandle;

	// BP에서 정의된 구성요소와 바인딩되어있다
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBox;

	// 알림 출력 시 애니메이션
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* TextPopUpAnim = nullptr;

	// 애니메이션 종료 시 콜백
	FWidgetAnimationDynamicEvent TextPopUpAnimEndEvent;

private:
	float CurrentWaitDuration = 0.0f;
};
