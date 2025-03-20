// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "PlayerTriggerVolume.h"
#include "LobbyFallVolume.generated.h"

/**
 * 오버랩 시 스폰 위치로 이동시킨다
 */
UCLASS()
class PROJECTTR_API ALobbyFallVolume : public APlayerTriggerVolume
{
	GENERATED_BODY()
	
protected:
	virtual void ProcessPlayerOverlap(class AFPSCharacter* Player, bool bAllPlayerOverlapped) override;
};
