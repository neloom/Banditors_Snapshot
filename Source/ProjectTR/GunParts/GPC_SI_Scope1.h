// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_SI_Scope1.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_SI_Scope1 : public UGunPartComponent
{
	GENERATED_BODY()

	UGPC_SI_Scope1();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Sight; }
};
