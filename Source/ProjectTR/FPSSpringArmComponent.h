// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "FPSSpringArmComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UFPSSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()
	
	UFPSSpringArmComponent();

protected:
	void BeginPlay();

public:
	// 스프링암의 길이를 Pitch에 따라 다르게 설정한다
	// 인자로 이 타겟암이 회전할 수 있는 최대, 최소 Pitch를 전달한다
	void SetTargetArmLengthByRotation(float MinPitch, float MaxPitch);

	/* Getters */
	const float GetOriginTargetArmLength() { return OriginTargetArmLength; }

private:
	float OriginTargetArmLength = 0.0f;
};
