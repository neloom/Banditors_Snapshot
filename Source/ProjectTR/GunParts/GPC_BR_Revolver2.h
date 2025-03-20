// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_BR_Revolver2.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_BR_Revolver2 : public UGunPartComponent
{
	GENERATED_BODY()
	
	UGPC_BR_Revolver2();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Barrel; }
};
