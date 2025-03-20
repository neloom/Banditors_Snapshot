// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "DungeonActor.h"
#include "Net/UnrealNetwork.h"
#include "MovingPlatform.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API AMovingPlatform : public ADungeonActor
{
	GENERATED_BODY()
	
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AMovingPlatform, CurrentLocation);
	}

public:
	AMovingPlatform();
	virtual void BeginPlay() override;
	virtual void Initialize() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	void ReachedMovePoint();

public:
	// 현재 활성화 여부
	UPROPERTY(EditAnywhere)
	bool bPlatformActivated = false;

protected:
	// 이동 목표의 델타
	// 시작점은 BeginPlay 시점에서의 액터 위치로 설정한다
	UPROPERTY(EditAnywhere)
	FVector MoveDeltaLocation;

	UPROPERTY(EditAnywhere)
	float Speed = 100.0f;

	// 지점 도착 후 대기 시간
	UPROPERTY(EditAnywhere)
	float PauseTime = 0.0f;

	// 현재 위치 트래킹; 클라이언트는 이 값을 사용해 Interpolate한다
	UPROPERTY(Replicated)
	FVector CurrentLocation;

	// 보간 속도
	UPROPERTY(EditAnywhere)
	float LocInterpSpeed = 1.0f;

private:
	// 현재 목표지점으로 향하고 있는지, 아니면 반대로 돌아오고 있는지 여부
	bool bMovingToTarget = true;

	// 현재 일시적으로 대기중인지 여부
	bool bPaused = false;

	// 이동 위치 정보
	FVector TargetLocation;
	FVector StartLocation;
	
	// 지점 대기 타이머
	FTimerHandle PauseTimer;
};
