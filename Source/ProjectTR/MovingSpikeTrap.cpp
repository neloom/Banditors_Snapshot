// Copyright (C) 2024 by Haguk Kim


#include "MovingSpikeTrap.h"
#include "Room.h"

AMovingSpikeTrap::AMovingSpikeTrap()
{
	// lerping을 위해서 해제
	SetReplicatingMovement(false);
	if (MeshComponent)
	{
		MeshComponent->SetIsReplicated(false);
	}
}

void AMovingSpikeTrap::BeginPlay()
{
	Super::BeginPlay();
}

void AMovingSpikeTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		ProcessMove(DeltaTime);
		CurrentLocation = GetActorLocation();
		CurrentRotation = GetActorRotation();
	}

	if (!HasAuthority())
	{
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), CurrentLocation, DeltaTime, LocInterpSpeed), false, nullptr, ETeleportType::None);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), CurrentRotation, DeltaTime, RotInterpSpeed), ETeleportType::None);
	}
}

void AMovingSpikeTrap::Initialize()
{
	Super::Initialize();
	if (HasAuthority())
	{
		if (bSpawnAsMoving) CurrentState = EMovingTrapState::MTS_Moving;

		StartLocation = GetActorLocation();
		StartRotation = GetActorRotation();

		// 중요: 델타 포지션을 적용할 때는 해당 값을 월드 스페이스로 변경해주어야 함
		if (OwningRoom)
		{
			LocationDelta = ConvertRoomToWorld(LocationDelta);
		}

		TargetLocation = StartLocation + LocationDelta;
		TargetRotation = StartRotation + RotationDelta;
	}

	// NOTE: 최초 CurrentLocation은 클라에서도 반드시 설정해주어야 함
	CurrentLocation = GetActorLocation();
}

void AMovingSpikeTrap::ProcessMove(float DeltaTime)
{
	if (bProcessMoveOnlyIfTriggered && !bTrapTriggered) return;
	switch (CurrentState)
	{
		case EMovingTrapState::MTS_Ready:
		{
			if (bMovedOnceOrMore && !bShouldRepeat) return;
			if (HoldReadyStateForDuration < CurrentStateTime)
			{
				CurrentStateTime = 0.0f;
				CurrentState = EMovingTrapState::MTS_Moving;
			}
			break;
		}
		case EMovingTrapState::MTS_Moving:
		{
			bMovedOnceOrMore = true;
			bool Finished = StartMoving(CurrentStateTime);
			if (Finished)
			{
				CurrentStateTime = 0.0f;
				CurrentState = EMovingTrapState::MTS_Finished;
			}
			break;
		}
		case EMovingTrapState::MTS_Finished:
		{
			if (HoldFinishedStateForDuration < CurrentStateTime)
			{
				CurrentStateTime = 0.0f;
				if (bShouldReverseBeforeRepeat)
				{
					CurrentState = EMovingTrapState::MTS_Reversing;
				}
				else
				{
					CurrentState = EMovingTrapState::MTS_Ready;
				}
			}
			break;
		}
		case EMovingTrapState::MTS_Reversing:
		{
			bool Finished = StartReversing(CurrentStateTime);
			if (Finished)
			{
				CurrentStateTime = 0.0f;
				CurrentState = EMovingTrapState::MTS_Ready;
			}
			break;
		}
	}

	if (bRotateLoopIndependently)
	{
		SetActorRotation(FMath::Lerp(TargetRotation, StartRotation, FMath::Clamp(CurrentIndependentRotTime / TargetIndependentRotationLoopTime, 0.f, 1.f)), ETeleportType::None);
		CurrentIndependentRotTime = CurrentIndependentRotTime + DeltaTime;
		if (CurrentIndependentRotTime > TargetIndependentRotationLoopTime) CurrentIndependentRotTime -= TargetIndependentRotationLoopTime;
	}
	CurrentStateTime += DeltaTime;
}

void AMovingSpikeTrap::OnBoxCollision(AActor* Target)
{
	if (bCollideOnlyDuringMovement && CurrentState != EMovingTrapState::MTS_Moving && CurrentState != EMovingTrapState::MTS_Reversing) return;
	return Super::OnBoxCollision(Target);
}

bool AMovingSpikeTrap::StartMoving(float MoveStateTime)
{
	float Alpha = FMath::Clamp(MoveStateTime / TargetMovingTime, 0.f, 1.f);
	if (bMoveLocation) SetActorLocation(FMath::Lerp(StartLocation, TargetLocation, Alpha), false, nullptr, ETeleportType::None);
	if (!bRotateLoopIndependently && bMoveRotation) SetActorRotation(FMath::Lerp(StartRotation, TargetRotation, Alpha), ETeleportType::None);
	return Alpha >= 1.f;
}

bool AMovingSpikeTrap::StartReversing(float ReverseStateTime)
{
	float Alpha = FMath::Clamp(ReverseStateTime / TargetMovingTime, 0.f, 1.f);
	if (bMoveLocation) SetActorLocation(FMath::Lerp(TargetLocation, StartLocation, Alpha), false, nullptr, ETeleportType::None);
	if (!bRotateLoopIndependently && bMoveRotation) SetActorRotation(FMath::Lerp(TargetRotation, StartRotation, Alpha), ETeleportType::None);
	return Alpha >= 1.f;
}

