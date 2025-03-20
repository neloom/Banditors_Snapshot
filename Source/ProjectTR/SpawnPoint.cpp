// Copyright (C) 2024 by Haguk Kim


#include "SpawnPoint.h"
#include "TRUtils.h"

ASpawnPoint::ASpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

}

FVector ASpawnPoint::GetRandomSpawnLocation() const
{
	FVector Result = GetActorLocation();
	if (bUseNavMesh)
	{
		Result = TRUtils::FindRandomNavigationPoint(GetWorld(), GetActorLocation(), Radius);
	}
	else
	{
		const FVector2D& DeltaPosition = FMath::RandPointInCircle(Radius);
		Result = GetActorLocation() + FVector(DeltaPosition, 0);
	}
	return Result;
}

void ASpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 32, FColor::Green, false, 0.1f);
#endif
}


