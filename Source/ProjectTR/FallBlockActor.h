// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "DungeonActor.h"
#include "FallBlockActor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API AFallBlockActor : public ADungeonActor
{
	GENERATED_BODY()
	
protected:
	virtual void OnTriggered() override;
};
