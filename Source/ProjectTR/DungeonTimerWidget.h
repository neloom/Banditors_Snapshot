// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "Components/TextBlock.h"
#include "DungeonTimerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UDungeonTimerWidget : public UTRWidget
{
	GENERATED_BODY()

public:
	void Update();

	UFUNCTION(BlueprintCallable)
	bool GetLocalTimerActive() { return bLocal_TimerActive; }

	UFUNCTION(BlueprintCallable)
	void SetLocalTimerActive(bool Value) { bLocal_TimerActive = Value; }

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TimerText;

protected:
	// 타이머 사용 여부
	// 위젯 자체의 Activate 여부와는 별개로, 내부 로직을 실행할지 말지를 결정한다
	bool bLocal_TimerActive = false;

public:
	FORCEINLINE UTextBlock* GetTimerText() { return TimerText; }
};
