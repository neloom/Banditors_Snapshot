// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRWidget.h"
#include "Components/TextBlock.h"
#include "DungeonTimerWidget.h"
#include "TRExpWidget.h"
#include "TRHealthBar.h"
#include "TRShardWidget.h"
#include "TRHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UTRHUDWidget : public UTRWidget
{
	GENERATED_BODY()
	
public:
	void SetTarget(class AGameCharacter* Target);

	// NOTE: 개별 항목 단위로 업데이트 가능

	// 연사속도가 레플리케이션보다 빠른 경우에도 부드러운 움직임을 처리하기 위해 클라 예측치를 UI에 적용하는 것을 허용함
	// 애초에 오차가 발생하지도 않으며, 설사 오차가 발생하더라도 최종적으로 레플리케이션에 의한 보정이 이뤄지므로 문제 없음
	// ClientAmmoPrediction이 0 혹은 양수인 경우에만 적용됨
	void UpdateAmmo(int32 ClientAmmoPrediction = -1);
	void UpdateSlot();
	void UpdateDungeonDepth();
	void UpdateDungeonTimer();
	void UpdateHealthBar();
	void UpdateExp();
	void UpdateShards();

	// 일괄 업데이트; 타깃 교체 시 자동으로 호출된다
	void UpdateAll();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* AmmoLeftText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* SlotText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* DungeonDepthText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UDungeonTimerWidget* WB_DungeonTimer = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTRHealthBar* WB_HealthBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTRExpWidget* WB_LevelExpBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTRShardWidget* WB_Shards = nullptr;
	
public:
	// UI가 나타내는 타깃 (바인딩된 타깃)
	class AGameCharacter* HudTarget = nullptr;
};
