// Copyright (C) 2024 by Haguk Kim


#include "MovingPlatform.h"
#include "TRMacros.h"
#include "TimerManager.h"

AMovingPlatform::AMovingPlatform()
{
    // lerping을 위해서 해제
    SetReplicatingMovement(false);

    if (MeshComponent)
    {
        MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);

        // NOTE: AI가 플랫폼 위로 추격을 할 수 있도록 하려면 true로 설정해야 함
        // 단 이 경우 메쉬를 실시간으로 수정해야 하기 때문에 런타임 오버헤드가 증가함
        MeshComponent->SetCanEverAffectNavigation(true);
        MeshComponent->SetIsReplicated(false);
    }
}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();
}

void AMovingPlatform::Initialize()
{
    Super::Initialize();
    StartLocation = GetActorLocation();
    if (OwningRoom)
    {
        MoveDeltaLocation = ConvertRoomToWorld(MoveDeltaLocation);
    }
    TargetLocation = StartLocation + MoveDeltaLocation;
    CurrentLocation = StartLocation; // NOTE: 최초 CurrentLocation은 클라에서도 반드시 설정해주어야 함
}

void AMovingPlatform::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(PauseTimer);
    }
}

void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (HasAuthority() && bPlatformActivated && !bPaused)
    {
        FVector Target = bMovingToTarget ? TargetLocation : StartLocation;
        FVector Direction = (Target - CurrentLocation).GetSafeNormal();
        float Distance = FVector::Dist(CurrentLocation, Target);

        if (Distance > Speed * DeltaTime)
        {
            CurrentLocation += Direction * Speed * DeltaTime;
        }
        else
        {
            CurrentLocation = Target;
            bMovingToTarget = !bMovingToTarget;

            if (PauseTime > 0)
            {
                bPaused = true;
                
                UWorld* World = GetWorld();
                if (World)
                {
                    World->GetTimerManager().SetTimer(PauseTimer, this, &AMovingPlatform::ReachedMovePoint, PauseTime, false);
                }
            }
        }

        SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::None);
    }

    if (!HasAuthority())
    {
        SetActorLocation(FMath::VInterpTo(GetActorLocation(), CurrentLocation, DeltaTime, LocInterpSpeed), false, nullptr, ETeleportType::None);
    }
}

void AMovingPlatform::ReachedMovePoint()
{
    bPaused = false;
}
