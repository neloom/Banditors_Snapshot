// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "PlayerTriggerVolume.h"
#include "TouchActivationTrap.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ATouchActivationTrap : public APlayerTriggerVolume
{
	GENERATED_BODY()
	
public:
	ATouchActivationTrap();

public:
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* MeshComponent = nullptr;
};
