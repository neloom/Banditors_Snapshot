// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "EscapableWidget.generated.h"

/**
 * Escape 키를 통해 닫을 수 있는 위젯을 나타낸다
 */
UCLASS()
class PROJECTTR_API UEscapableWidget : public UTRWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	// 탈출에 사용할 수 있는 키들의 목록
	UPROPERTY(EditDefaultsOnly)
	TArray<FKey> EscapeKeys = { EKeys::Escape };
};
