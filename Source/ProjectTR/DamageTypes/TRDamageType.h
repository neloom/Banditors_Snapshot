// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "TRDamageType.generated.h"

/**
 * NOTE: DamageType은 Statefull해서는 안되며 인스턴스가 아닌 static한 정보와 기능만을 담아야 한다
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTTR_API UTRDamageType : public UDamageType
{
	GENERATED_BODY()
	
};
