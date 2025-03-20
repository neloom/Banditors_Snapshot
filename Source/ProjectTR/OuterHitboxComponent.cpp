// Copyright (C) 2024 by Haguk Kim


#include "OuterHitboxComponent.h"
#include "TRMacros.h"
#include "GameCharacter.h"

UOuterHitboxComponent::UOuterHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName("OuterHitbox");
	SetGenerateOverlapEvents(false);
	SetShouldUpdatePhysicsVolume(false);
	SetSimulatePhysics(false);
}

void UOuterHitboxComponent::OnOuterHitboxCollision(float HitboxDuration)
{
	AGameCharacter* HitboxOwner = Cast<AGameCharacter>(GetOwner());
	if (HitboxOwner)
	{
		if (HitboxDuration <= UE_SMALL_NUMBER)
		{
			HitboxOwner->ActivateDetailHitboxForTick();
		}
		else
		{
			HitboxOwner->ActivateDetailedHitboxFor(HitboxDuration);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OnOuterHitboxCollision - Hitbox has no valid owner!"));
	}
}
