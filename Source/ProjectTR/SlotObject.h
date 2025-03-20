// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "InvObject.h"
#include "SlotObject.generated.h"

/**
 * UInvObject를 Downcast해서 사용하므로, 별도의 데이터를 추가하는 것은 권장하지 않는다
 */
UCLASS()
class PROJECTTR_API USlotObject : public UInvObject
{
	GENERATED_BODY()
};
