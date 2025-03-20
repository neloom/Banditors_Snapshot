// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "LocalActor.h"
#include "DamageNumberWidget.h"
#include "Components/WidgetComponent.h"
#include "LocalDamageNumber.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ALocalDamageNumber : public ALocalActor
{
	GENERATED_BODY()
	
public:
	ALocalDamageNumber();
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	void SetWidgetClassAs(TSubclassOf<UDamageNumberWidget> WidgetClass);

	void StartDisplay(int32 Damage);
	void StopDisplay(bool bClearWidgetData);
	UWidgetComponent* GetWidgetComp() { return Cast<UWidgetComponent>(RootComponent); }

	// 큐가 기준치 이상으로 커지는 것을 방지하기 위해 현재 유효한 개체들의 수를 트래킹한다
	static int32 CurrInstCount;
	static const int32 MaxInstCount = 32;

protected:
	UFUNCTION()
	void OnUIAnimEnd();

public:
	// 사용 완료 시 돌아갈 풀; 유효하지 않은 경우 풀로 반환되는 대신 액터를 파괴한다
	TQueue<ALocalDamageNumber*>* ReturnPool = nullptr;

protected:
	UPROPERTY(BlueprintReadOnly)
	int32 DamageValue = 0;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UDamageNumberWidget> DamageNumberWidgetClass = nullptr;

	float EstimatedDisplayTime = 0.0f;

	FTimerHandle DisplayTimer;
};
