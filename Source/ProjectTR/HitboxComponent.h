// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "HitboxComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UHitboxComponent : public UBoxComponent
{
	GENERATED_BODY()
	
	UHitboxComponent();

public:
	/* Getters */
	float GetDamageMultiplier() { return DamageMultiplier; }

	/* Setters */
	void SetDamageMultiplier(float Value) { DamageMultiplier = Value; }

protected:
	// 이 히트박스에 대한 데미지 증가폭
	// NOTE: GameCharacter에서 수정하는 것이 권장됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;
};
