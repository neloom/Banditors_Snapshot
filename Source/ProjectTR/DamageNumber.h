// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "UserSpecificWidgetActor.h"
#include "DamageNumberWidget.h"
#include "Net/UnrealNetwork.h"
#include "DamageNumber.generated.h"

/**
 * Deprecated; LocalDamageNumber를 사용할 것
 */
UCLASS()
class PROJECTTR_API ADamageNumber : public AUserSpecificWidgetActor
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		// 데미지 값은 최초 1회만 레플리케이션한다
		DOREPLIFETIME_CONDITION(ADamageNumber, DamageValue, COND_InitialOnly);
	}

public:
	ADamageNumber();
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void ClearData() override;

	// Setters
	void SetDamageValue(int32 Value) { DamageValue = Value; }

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageNumberWidget> DamageNumberWidgetClass = nullptr;

protected:
	UPROPERTY(Replicated)
	int32 DamageValue = 0;
};
