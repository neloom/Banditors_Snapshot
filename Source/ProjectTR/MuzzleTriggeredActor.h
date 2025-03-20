// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "DungeonActor.h"
#include "MuzzleTriggeredActor.generated.h"

/**
 * 캐릭터의 머즐로부터의 Ray를 기반으로 트리거 발동을 처리하는 액터이다
 * 문, 손잡이 등 플레이어 상호작용이 가능한 대부분의 액터가 여기에 해당된다
 */
UCLASS(Abstract)
class PROJECTTR_API AMuzzleTriggeredActor : public ADungeonActor
{
	GENERATED_BODY()
	
public:
	AMuzzleTriggeredActor();
	virtual void BeginPlay() override;
	void MuzzleTrigger(class AGameCharacter* TriggeredBy);

protected:
	// 캐릭터의 시야 방향에 놓여 트리거 되었을 때의 로직
	virtual void OnMuzzleTriggered(class AGameCharacter* TriggeredBy);

	// TODO: 트리거 범위에 속할 경우 아웃라인 그리기

protected:
	// 머즐 raycast 트리거 범위 판정을 위해 사용하는 박스
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* TriggerBox = nullptr;
};
