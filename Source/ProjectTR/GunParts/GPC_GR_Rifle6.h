// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_GR_Rifle6.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_GR_Rifle6 : public UGunPartComponent
{
	GENERATED_BODY()
	
	UGPC_GR_Rifle6();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Grip; }
};
