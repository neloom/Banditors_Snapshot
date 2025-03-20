// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "FirstPersonMeshComponent.generated.h"

/**
 * 게임플레이 카메라에 비춰지는 1인칭 모델
 */
UCLASS()
class PROJECTTR_API UFirstPersonMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	
public:
	UFirstPersonMeshComponent();
};
