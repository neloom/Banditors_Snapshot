// Copyright (C) 2025 by Haguk Kim


#include "TriggerOpenDoor.h"

ATriggerOpenDoor::ATriggerOpenDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // RPC 사용 필요
	SetReplicatingMovement(false);
}

void ATriggerOpenDoor::BeginPlay()
{
	Super::BeginPlay();
	// 명시적으로 틱을 활성화 하지 않을 경우 틱이 해제된 상태로 소환될 수 있음
	SetActorTickEnabled(true);
}

void ATriggerOpenDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bLocal_StartMoving)
	{
		float Alpha = FMath::Clamp(Local_CurrMoveTime / MoveTime, 0.0f, 1.0f);
		SetActorLocation(FMath::Lerp(Local_StartLocation, Local_TargetLocation, Alpha), false, nullptr, ETeleportType::None);
		SetActorRotation(FMath::Lerp(Local_StartRotation, Local_TargetRotation, Alpha), ETeleportType::None);
		Local_CurrMoveTime += DeltaTime;

		// 이동 완료
		if (Alpha >= 1.0f)
		{
			bLocal_StartMoving = false;
			Local_CurrMoveTime = 0.0f;
		}
	}
}

void ATriggerOpenDoor::OnTriggered()
{
	Super::OnTriggered();
	if (HasAuthority() && !bServer_Opened)
	{
		bServer_Opened = true;
		Multicast_OpenDoor();

		// NOTE: 현재는 닫기 로직은 없음
	}
}

void ATriggerOpenDoor::OnMuzzleTriggered(AGameCharacter* TriggeredBy)
{
	if (bTriggerLogicWhenMuzzleTriggered)
	{
		TriggerThis();
	}
}

void ATriggerOpenDoor::Local_OpenDoor()
{
	bLocal_StartMoving = true;
	Local_StartLocation = GetActorLocation();
	Local_StartRotation = GetActorRotation();
	Local_TargetLocation = Local_StartLocation + LocationDelta;
	Local_TargetRotation = Local_StartRotation + RotationDelta;

	// 문이 열리기 시작한 이후부터는 콜리전을 해제한다
	// 문 움직임 동기화 방식은 어쩔 수 없이 서버와 클라 싱크차가 항상 발생하기 때문에,
	// 움직이는 동안 문이 물리적으로 영향을 주는 것 자체가 바람직하지 않다
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATriggerOpenDoor::Multicast_OpenDoor_Implementation()
{
	Local_OpenDoor();
}
