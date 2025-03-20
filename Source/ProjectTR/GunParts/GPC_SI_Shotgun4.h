// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_SI_Shotgun4.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_SI_Shotgun4 : public UGunPartComponent
{
	GENERATED_BODY()

	UGPC_SI_Shotgun4();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Sight; }
};
