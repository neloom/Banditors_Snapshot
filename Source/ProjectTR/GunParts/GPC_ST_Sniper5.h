// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_ST_Sniper5.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_ST_Sniper5 : public UGunPartComponent
{
	GENERATED_BODY()
	
	UGPC_ST_Sniper5();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Stock; }
};
