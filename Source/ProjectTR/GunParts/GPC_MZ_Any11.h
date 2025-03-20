// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_MZ_Any11.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_MZ_Any11 : public UGunPartComponent
{
	GENERATED_BODY()

	UGPC_MZ_Any11();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Muzzle; }
};
