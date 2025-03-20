// Copyright (C) 2024 by Haguk Kim


#include "TRProjMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameCharacter.h"
#include "TRMacros.h"
#include "HitboxComponent.h"

UTRProjMovementComponent::UTRProjMovementComponent()
{
    PrimaryComponentTick.TickInterval = TR_PROJ_MOVEMENT_TICKRATE;
}

UProjectileMovementComponent::EHandleBlockingHitResult UTRProjMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
    if (bUseDefaultHitHandlers)
    {
        return Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
    }
	return EHandleBlockingHitResult::Abort; // NOTE: AdvanceNextSubstep 사용 시 불필요한 부하 증가
}

void UTRProjMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	if (bUseDefaultHitHandlers)
	{
		return Super::HandleImpact(Hit, TimeSlice, MoveDelta);
	}

	// 아무 것도 처리하지 않음
	return;
}

void UTRProjMovementComponent::PredictProjHitUsingSphereTrace(float TraceDist, float PredictSphereRadius, TArray<AActor*>& ActorsToIgnore, AGameCharacter* HitboxOwner, UHitboxComponent*& out_PredictedHitTarget)
{
	if (!HitboxOwner || TraceDist <= 0.0f) return;

    FVector TraceStart = UpdatedComponent->GetComponentLocation();
    FVector TraceEnd = TraceStart + (Velocity.GetSafeNormal() * TraceDist);
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Hitbox));

    FHitResult Hit;
    bool bHit = false;
    bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
        HitboxOwner->GetWorld(),
        TraceStart,
        TraceEnd,
        PredictSphereRadius,
        ObjectTypes,
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        Hit,
        true /* 자신 무시 */
    );

    // 설사 히트가 발생했더라도 히트박스의 주인이 원하는 타깃과 다르다면 무시한다
    UHitboxComponent* HitComp = Cast<UHitboxComponent>(Hit.GetComponent());
    if (bHit && HitComp && HitComp->GetOwner() == HitboxOwner)
    {
        out_PredictedHitTarget = HitComp;
    }
    return;
}
