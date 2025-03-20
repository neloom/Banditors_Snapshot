// Copyright (C) 2024 by Haguk Kim


#include "TouchActivationTrap.h"

ATouchActivationTrap::ATouchActivationTrap()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	check(MeshComponent);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetShouldUpdatePhysicsVolume(false);
}

