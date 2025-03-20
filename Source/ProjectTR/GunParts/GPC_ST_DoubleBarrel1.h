// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_ST_DoubleBarrel1.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_ST_DoubleBarrel1 : public UGunPartComponent
{
	GENERATED_BODY()
	
	UGPC_ST_DoubleBarrel1();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Stock; }
};
