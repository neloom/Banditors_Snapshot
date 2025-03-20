// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "FPSCameraComponent.generated.h"

/**
 * 1인칭 시점 카메라
 */
UCLASS()
class PROJECTTR_API UFPSCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	UFPSCameraComponent();
};
