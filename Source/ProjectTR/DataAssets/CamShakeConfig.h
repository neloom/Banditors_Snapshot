// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TRCameraShake.h"
#include "CamShakeConfig.generated.h"

UENUM(BlueprintType)
enum class ECamShakeReference : uint8
{
	ECR_NULL,
	ECR_OnFireCamShake,
	ECR_OnDamageCamShake,
};

/**
 * 
 */
UCLASS()
class PROJECTTR_API UCamShakeConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Enum 기반 에셋 서칭
	TSubclassOf<UTRCameraShake> SearchCameraShakeFromEnum(ECamShakeReference CamShakeRef) const;

/* Camera Shake */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UTRCameraShake> OnFireCamShake = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UTRCameraShake> OnDamageCamShake = nullptr;
};
