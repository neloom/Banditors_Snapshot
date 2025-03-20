// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_SI_Sniper8.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_SI_Sniper8 : public UGunPartComponent
{
	GENERATED_BODY()

	UGPC_SI_Sniper8();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Sight; }
};
