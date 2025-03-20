// Copyright (C) 2024 by Haguk Kim


#include "FPSSpringArmComponent.h"

UFPSSpringArmComponent::UFPSSpringArmComponent()
{
	bUsePawnControlRotation = true; // 회전 상속
	bInheritPitch = true;
	bInheritRoll = true;
	bInheritYaw = true;
	bEnableCameraLag = false;
	TargetArmLength = 0.0f;
}

void UFPSSpringArmComponent::BeginPlay()
{
	OriginTargetArmLength = TargetArmLength;
}

void UFPSSpringArmComponent::SetTargetArmLengthByRotation(float MinPitch, float MaxPitch)
{
	float CurrPitch = GetTargetRotation().Pitch;
	if (CurrPitch >= 180) CurrPitch -= 360; // 아래: -90 위: 90으로
	float Alpha = (CurrPitch - MinPitch) / (MaxPitch - MinPitch);
	TargetArmLength = FMath::Lerp<float, float>(0.0f, OriginTargetArmLength, Alpha);
	//UE_LOG(LogTemp, Error, TEXT("%f (%f - %f) / (%f - %f)"), Alpha, CurrPitch, MinPitch, MaxPitch, MinPitch);
}
