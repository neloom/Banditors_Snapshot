// Copyright (C) 2024 by Haguk Kim


#include "BulletTraceProjectile.h"
#include "TRProjMovementComponent.h"
#include "Components/SphereComponent.h"
#include "FxConfig.h"

ABulletTraceProjectile::ABulletTraceProjectile()
{
	EnemyDamage = 0.0f;
	AllyDamage = 0.0f;
	DestroyOnHitCount = 1; // 충돌 직후 파괴
	ImpactMass = 0.0f;
	bShouldAffectForceOnCollision = false;

	bUseHitNormalForVFX = false;

	if (ProjectileMovementComponent)
	{
		InitialLifeSpan = 1.0f;
		ProjectileMovementComponent->InitialSpeed = 8000.0f;
		ProjectileMovementComponent->MaxSpeed = 8000.0f;
		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		ProjectileMovementComponent->bShouldBounce = false;
		ProjectileMovementComponent->Bounciness = 0.0f;
		ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	}
}

void ABulletTraceProjectile::Local_InitializeVFX()
{
	// 바인딩 안되어있을 경우 기본 총알 Trace로 trail vfx 바인딩
	if (!TrailVFX && FxConfig)
	{
		TrailVFX = FxConfig->BLT_Default;
	}

	// 나머지 초기화 진행
	Super::Local_InitializeVFX();
}
