// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "SpikeTrap.h"
#include "Net/UnrealNetwork.h"
#include "MovingSpikeTrap.generated.h"

UENUM(BlueprintType)
enum class EMovingTrapState : uint8
{
	MTS_Ready UMETA(DisplayName = "Ready"),
	MTS_Moving UMETA(DisplayName = "Moving"),
	MTS_Finished UMETA(DisplayName = "Finished"),
	MTS_Reversing UMETA(DisplayName = "Reversing")
};

/**
 * 
 */
UCLASS()
class PROJECTTR_API AMovingSpikeTrap : public ASpikeTrap
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AMovingSpikeTrap, CurrentLocation);
		DOREPLIFETIME(AMovingSpikeTrap, CurrentRotation);
	}

public:
	AMovingSpikeTrap();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Initialize() override;

protected:
	void ProcessMove(float DeltaTime);
	virtual void OnBoxCollision(AActor* Target) override;

	// 이동 로직을 처리한다; 이동 완료 여부를 반환한다
	bool StartMoving(float MoveStateTime);
	bool StartReversing(float ReverseStateTime);

private:
	// 이 값을 설정하는 것으로 이동 상태의 변경이 가능
	EMovingTrapState CurrentState = EMovingTrapState::MTS_Ready;

	// 한 번 이상 이동을 처리했는지 여부
	bool bMovedOnceOrMore = false;

	// 현재 state의 지속 시간
	float CurrentStateTime = 0.0f;

	// Independent rotation 사용 시 현재 회전 지속 시간
	float CurrentIndependentRotTime = 0.0f;

	// 현재 목표 위치 및 회전
	FVector StartLocation;
	FRotator StartRotation;
	FVector TargetLocation;
	FRotator TargetRotation;

	// 클라이언트 동기화에 사용
	UPROPERTY(Replicated)
	FVector CurrentLocation;

	UPROPERTY(Replicated)
	FRotator CurrentRotation;

	UPROPERTY(EditAnywhere)
	float LocInterpSpeed = 1.0f;

	UPROPERTY(EditAnywhere)
	float RotInterpSpeed = 1.0f;

protected:
	// 이동 델타 거리
	UPROPERTY(EditAnywhere)
	FVector LocationDelta;

	UPROPERTY(EditAnywhere)
	bool bMoveLocation = false;

	// 이동 델타 회전
	UPROPERTY(EditAnywhere)
	FRotator RotationDelta;

	UPROPERTY(EditAnywhere)
	bool bMoveRotation = false;

	// 생성 시점부터 이동을 개시할 지 여부
	// 이 값이 true일 경우 Hold 시간은 무시됨
	UPROPERTY(EditAnywhere)
	bool bSpawnAsMoving = false;

	// 이동 개시가 완료된 이후 다시 이동을 개시할지 여부
	UPROPERTY(EditAnywhere)
	bool bShouldRepeat = false;

	// 이동을 두 번 이상 개시할 때 그 전 이동을 롤백한 후에 이동을 처리할 지 여부 (e.g. 가시 왕복)
	UPROPERTY(EditAnywhere)
	bool bShouldReverseBeforeRepeat = false;

	// 이동 완료 시점에서 몇 초간 대기할 지 여부
	UPROPERTY(EditAnywhere)
	float HoldFinishedStateForDuration = 0.0f;

	// 이동 준비 시점에서 몇 초간 대기할 지 여부
	// bSpawnAsMoving이 true일 경우 최초 실행 시에는 무시됨
	UPROPERTY(EditAnywhere)
	float HoldReadyStateForDuration = 0.0f;

	// 이동 및 복귀에 소요되는 총 시간 (델타 타임에 따라 실제 소요시간이 약간 더 클 수 있음)
	UPROPERTY(EditAnywhere)
	float TargetMovingTime = 0.0f;

	// 참일 경우 이동 시퀀스와 별개로 회전을 처리하며, 이 경우 회전 한번을 처리하기 위해 TargetRotationLoopTime을 사용한다
	UPROPERTY(EditAnywhere)
	bool bRotateLoopIndependently = false;

	UPROPERTY(EditAnywhere)
	float TargetIndependentRotationLoopTime = 0.0f;

	// 회전 칼날 등의 경우 회전하는 경우에만 데미지를 가하게 만들 수 있다
	UPROPERTY(EditAnywhere)
	bool bCollideOnlyDuringMovement = false;

	// 항상 움직임을 처리할 지, 트리거 이후에만 처리할 지 여부
	UPROPERTY(EditAnywhere)
	bool bProcessMoveOnlyIfTriggered = false;
};
