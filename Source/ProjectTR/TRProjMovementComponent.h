// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TRProjMovementComponent.generated.h"

/**
 * UTRProjMovementComponent의 주 목적은 투사체 관통을 처리하기 위함이다
 * 투사체 관통을 위해서 Blocking hit와 Impact 연산을 무시하도록 기존 파이프라인을 오버라이드 하는 형태로 구현된다
 * 단 바운스하는 투사체의 경우에는 불가피하게 기존 파이프라인 함수를 사용해야 하는데(bUseDefaultHitHandlers = true),
 * 이러한 구조 때문에 캐릭터를 관통하면서 벽에는 바운스하는 투사체는 존재할 수 없다
 */
UCLASS()
class PROJECTTR_API UTRProjMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	UTRProjMovementComponent();
	
protected:
	virtual UProjectileMovementComponent::EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

public:
	// 현재 위치에서 속도 방향으로 주어진 거리 동안 트레이스해, 히트박스들 충돌 연산을 예측해 결과를 즉시 반환한다
	// 이때 주어진 Owner가 아닌 다른 히트박스와의 충돌이 먼저 이루어진 경우 결과 반환을 생략한다
	void PredictProjHitUsingSphereTrace(float TraceDist, float PredictSphereRadius, TArray<AActor*>& ActorsToIgnore, class AGameCharacter* HitboxOwner, class UHitboxComponent*& out_PredictedHitTarget);

public:
	// 블로킹 시 멈추는 기본 핸들러를 사용할지 여부
	UPROPERTY(EditDefaultsOnly)
	bool bUseDefaultHitHandlers = false;
};
