// Copyright (C) 2024 by Haguk Kim


#include "TriggerMoveActor.h"
#include "TRMacros.h"

ATriggerMoveActor::ATriggerMoveActor()
{
}

void ATriggerMoveActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority()) return;
	if (bIsMoving)
	{
		const FVector& NewLocation = FMath::VInterpConstantTo(GetActorLocation(), MoveTargetLocation, DeltaTime, MoveInterpSpeed);
		SetActorLocation(NewLocation);
		if (GetActorLocation().Equals(MoveTargetLocation))
		{
			// 이동 중지
			bIsMoving = false;
		}
	}
}

void ATriggerMoveActor::OnTriggered()
{
	if (!HasAuthority()) return;
	bIsMoving = true; // 이동 시작
	if (!bHasTriggeredBefore)
	{
		// 목표 지점은 최초 1회 설정
		MoveTargetLocation = GetActorLocation() + RelativeMoveDistance;
		bHasTriggeredBefore = true;
	}
}