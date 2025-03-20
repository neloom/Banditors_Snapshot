// Copyright (C) 2024 by Haguk Kim


#include "CamShakeConfig.h"

TSubclassOf<UTRCameraShake> UCamShakeConfig::SearchCameraShakeFromEnum(ECamShakeReference CamShakeRef) const
{
	switch (CamShakeRef)
	{
	case ECamShakeReference::ECR_NULL:
		return nullptr;
	case ECamShakeReference::ECR_OnFireCamShake:
		return OnFireCamShake;
	case ECamShakeReference::ECR_OnDamageCamShake:
		return OnDamageCamShake;
	default:
		UE_LOG(LogTemp, Error, TEXT("SearchCameraShakeFromEnum - Unable to find enum! %d"), CamShakeRef);
		return nullptr;
	}
	return nullptr;
}
