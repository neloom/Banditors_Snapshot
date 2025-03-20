// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "MuzzleTriggeredActor.h"
#include "TriggerOpenDoor.generated.h"

/**
 * TriggerOpenDoor의 경우 무브먼트를 매 틱마다 실시간 트래킹 해줄 필요가 없기 때문에,
 * MovingSpikeTrap과는 다른 방식으로, 단발성 RPC를 통해 이동을 처리한다
 * 이 방식을 사용하는 것으로 네트워크 부하를 줄일 수 있으나, 서버가 액터의 트랜스폼 수정을 마음대로 처리할 수 없다
 */
UCLASS()
class PROJECTTR_API ATriggerOpenDoor : public AMuzzleTriggeredActor
{
	GENERATED_BODY()
	
public:
	ATriggerOpenDoor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnTriggered() override;
	virtual void OnMuzzleTriggered(class AGameCharacter* TriggeredBy) override;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OpenDoor();
	void Local_OpenDoor();

protected:
	// 로컬 단위에서 가해지는 델타 값이기 때문에 초기 레플리케이션이 불필요하다
	UPROPERTY(EditAnywhere)
	FVector LocationDelta;

	UPROPERTY(EditAnywhere)
	FRotator RotationDelta;

	UPROPERTY(EditAnywhere)
	float MoveTime = 0.0f;

	// 머즐 트리거 시 로직 트리거를 처리할 지 여부
	// 만약 머즐 트리거가 아닌 어떤 특정 요건을 만족해야만 로직을 가동하게 할 경우에는 false로 설정하면 된다 (e.g. 보스룸의 출구 문, 잠긴 문 등)
	UPROPERTY(EditAnywhere)
	bool bTriggerLogicWhenMuzzleTriggered = true;

private:
	bool bServer_Opened = false;

	// 현재 타깃 트랜스폼으로 이동해야 하는지 여부
	bool bLocal_StartMoving = false;

	// 현재 이동에 사용한 시간
	float Local_CurrMoveTime = 0.0f;

	FVector Local_StartLocation;
	FRotator Local_StartRotation;
	FVector Local_TargetLocation;
	FRotator Local_TargetRotation;
};
