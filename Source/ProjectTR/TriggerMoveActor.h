// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "DungeonActor.h"
#include "TriggerMoveActor.generated.h"

/**
 * 트리거 될 경우 주어진 정보에 의해 선형적으로 움직인다
 */
UCLASS()
class PROJECTTR_API ATriggerMoveActor : public ADungeonActor
{
	GENERATED_BODY()

	ATriggerMoveActor();
	
protected:
	virtual void OnTriggered() override;

	void Tick(float DeltaTime);

protected:
	// 이 액터를 어떤 방향으로 얼마만큼 움직일지 표기
	UPROPERTY(EditAnywhere)
	FVector RelativeMoveDistance;

	// 이동 시 Interpolation 속도 (unit/sec)
	UPROPERTY(EditAnywhere)
	float MoveInterpSpeed = 200.0f;

	// 이동 목표 위치
	FVector MoveTargetLocation;

	// 현재 이동 중인지 여부
	// true로 설정될 경우 이동 로직을 처리한다
	bool bIsMoving = false;

	// 한 번만 트리거해야 하는 경우 사용한다
	bool bHasTriggeredBefore = false;
};
