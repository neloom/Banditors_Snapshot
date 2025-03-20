// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GunPartComponent.h"
#include "GPC_RC_SMG3.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UGPC_RC_SMG3 : public UGunPartComponent
{
	GENERATED_BODY()
	
	UGPC_RC_SMG3();

public:
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Receiver; }
};
