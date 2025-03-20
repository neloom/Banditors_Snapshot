// Copyright (C) 2024 by Haguk Kim


#include "HitboxComponent.h"
#include "TRMacros.h"

UHitboxComponent::UHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetComponentTickEnabled(false);

	SetCollisionProfileName("Hitbox");
	SetGenerateOverlapEvents(false);
	SetSimulatePhysics(false);
	SetShouldUpdatePhysicsVolume(false);
}