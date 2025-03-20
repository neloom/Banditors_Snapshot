// Copyright (C) 2024 by Haguk Kim


#include "RangedAttackComponent.h"


URangedAttackComponent::URangedAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URangedAttackComponent::BeginPlay()
{
	Super::BeginPlay();

}

void URangedAttackComponent::Server_FireProjectile(const FVector& SpawnLocation, const FVector& TargetLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("URangedAttackComponent::Server_FireProjectile - Invalid world!"));
		return;
	}
	TSubclassOf<ABaseProjectile> CurrFireProjClass = SelectFireProjectile();
	if (CurrFireProjClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		APawn* FirePawn = Cast<APawn>(GetOwner());
		if (FirePawn)
		{
			SpawnParams.Instigator = FirePawn->GetInstigator();
		}
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FVector& DesiredDirection = (TargetLocation - SpawnLocation);
		ABaseProjectile* SpawnProj = World->SpawnActor<ABaseProjectile>(CurrFireProjClass, SpawnLocation, DesiredDirection.Rotation(), SpawnParams);

		if (SpawnProj)
		{
			Multicast_OnFireProjectile();
		}
	}
}

TSubclassOf<ABaseProjectile> URangedAttackComponent::SelectFireProjectile()
{
	if (FireProjectileClass.IsEmpty()) return nullptr;
	return FireProjectileClass[0];
}

void URangedAttackComponent::Multicast_OnFireProjectile_Implementation()
{
	// 필요 시 구현
}


