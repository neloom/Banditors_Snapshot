// Copyright (C) 2024 by Haguk Kim


#include "WeaponFireComponent.h"
#include "FPSCharacter.h"
#include "GameCharacter.h"
#include "TimerManager.h"
#include "TRMacros.h"
#include "TRHUDWidget.h"
#include "TRUtils.h"
#include "FxConfig.h"
#include "CamShakeConfig.h"
#include "GunItem.h"
#include "BaseProjectile.h"
#include "BulletTraceProjectile.h"
#include "HitboxComponent.h"
#include "OuterHitboxComponent.h"
#include "ProjectTRGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "TRDamageType.h"
#include "TRProjMovementComponent.h"

UWeaponFireComponent::UWeaponFireComponent()
{
	// Replication 사용
	SetIsReplicatedByDefault(true);

	RapidFireDelegate = FTimerDelegate::CreateUObject(this, &UWeaponFireComponent::Fire);
	Local_RapidFxDelegate = FTimerDelegate::CreateUObject(this, &UWeaponFireComponent::Local_Fx);
}

bool UWeaponFireComponent::Host_CanTrigger(AGameCharacter* Invoker)
{
	if (!Super::Host_CanTrigger(Invoker)) return false;
	
	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	if (GunOwner)
	{
		// NOTE: CanTrigger에서 확인하는 것 만으로는 연사 도중 탄이 바닥나는 경우에 대한 처리를 해줄 수 없음
		return GunOwner->Host_CanFireShot();
	}
	return false;
}

void UWeaponFireComponent::SetupVirtualTargetDistance(const FVector& StartPos, const FVector& Direction, AActor* FireActor)
{
	FVector EndPos = StartPos + Direction * MaxTargetRange;
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = false;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(GetOwner());
	TraceParams.AddIgnoredActor(FireActor);

	// 가상타깃 설정 전에 발사 궤적에 외곽 히트박스가 있다면 활성화한다
	// NOTE: 성능을 위해 재귀는 처리하지 않는다
	FHitResult HitboxHitRes;
	if (GetWorld()->LineTraceSingleByProfile(HitboxHitRes, StartPos, EndPos, TEXT("HitscanProfile"), TraceParams))
	{
		UOuterHitboxComponent* OuterHitboxComp = Cast<UOuterHitboxComponent>(HitboxHitRes.Component);
		if (OuterHitboxComp)
		{
			OuterHitboxComp->OnOuterHitboxCollision(0.0f/*1틱*/);
		}
	}

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByProfile(HitResult, StartPos, EndPos, TEXT("VirtualTargetProfile"), TraceParams);
	if (bHit)
	{
		CurrVirtualTargetDistance = FVector::Dist(StartPos, HitResult.ImpactPoint);
	}
	else
	{
		CurrVirtualTargetDistance = MaxTargetRange;
	}
}

void UWeaponFireComponent::Fire()
{
	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	if (!GunOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponFireComponent::Fire - Invalid Actor Owner!"));
		return;
	}

	// 발사 가능한지 확인
	// NOTE: 트리거 전 확인하는 것과는 다른 목적성을 가짐
	// 연발 시 한번의 트리거 요청으로 여러번의 Fire()가 호출되기 때문에
	// 도중에 탄약이 바닥나는 등의 상황에 대해 예외 처리를 해줘야 함
	if (!GunOwner->Host_CanFireShot())
	{
		StopFire(CurrFireActor, CurrFireMode);
		return;
	}

	Host_SetupFireFromMuzzle(CurrFireActor);
	
	switch (CurrGunType)
	{
	case EWeaponFireType::WFT_PROJECTILE:
		FireProjectile();
		break;
	case EWeaponFireType::WFT_HITSCAN:
		FireHitscan();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("UWeaponFireComponent::Fire - Unknown fire type!"));
	}

	// 매 탄 발사 이후 처리
	GunOwner->OnShotFired();
	return;
}

void UWeaponFireComponent::OnRep_RecoilAnimInfluencer() const
{
	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	if (GunOwner)
	{
		GunOwner->RefreshRecoilAnim();
	}
}

void UWeaponFireComponent::OnPerHitscanHitboxCollision(UHitboxComponent* HitboxComp, FVector NormalImpulse, const FHitResult& Hit, AGunItem* GunOwner, AGameCharacter* Shooter, bool bIgnoreColVFX)
{
	if (!HitboxComp)
	{
		UE_LOG(LogTemp, Error, TEXT("OnHitscanHitboxCollision should not be called upon non-hitbox component collision!"));
		return;
	}
	if (!GunOwner || !Shooter)
	{
		UE_LOG(LogTemp, Error, TEXT("OnHitscanHitboxCollision - invalid argument!"));
		return;
	}
	AGameCharacter* HitboxOwner = Cast<AGameCharacter>(HitboxComp->GetOwner());
	if (!HitboxOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("OnHitscanHitboxCollision - invalid hitbox owner!"));
		return;
	}

	// VFX
	if (!bIgnoreColVFX && FireBulletStruct.HitCharFx)
	{
		Server_SpawnBulletHitCharacterVFX(Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	// 상태이상 적용
	const TArray<FStatEffectGenInfo>& StatEffectsOnHit = TRUtils::IsAllyWith(Shooter, HitboxOwner) ? StatEffsToAllyWhenHit : StatEffsToEnemyWhenHit;
	TRUtils::ApplyStatusEffectsOnTarget(StatEffectsOnHit, HitboxOwner, Shooter);

	// 데미지 부여
	float HitboxDmgMultiplier = HitboxComp->GetDamageMultiplier();

	float Damage = 0.0f;
	if (TRUtils::IsAllyWith(HitboxOwner, Shooter))
	{
		Damage = HitboxDmgMultiplier * GunOwner->GetStat_DmgAllyDirect(Shooter);
	}
	else
	{
		Damage = HitboxDmgMultiplier * GunOwner->GetStat_DmgEnemyDirect(Shooter);
	}

	// 부위별 보너스 적용
	if (HitboxComp == HitboxOwner->HeadColComponent)
	{
		Damage *= GunOwner->GetStat_DmgMultOnHead(Shooter);
	}

	// 에어샷 판정 적용
	if (HitboxOwner->IsInAir())
	{
		Damage *= GunOwner->GetStat_DmgMultOnAirshot(Shooter);
	}

	// 거리 배율 적용
	float TargetShooterDist = HitboxOwner->GetDistanceTo(Shooter);
	if (TargetShooterDist <= TR_GUN_CLOSE_DIST)
	{
		Damage *= GunOwner->GetStat_DmgMultDistClose(Shooter);
	}
	else if (TargetShooterDist >= TR_GUN_LONG_DIST)
	{
		Damage *= GunOwner->GetStat_DmgMultDistFar(Shooter);
	}

	// 거리 Falloff 적용
	if (GunOwner->GetStat_HasDmgDistFallOff(Shooter))
	{
		Damage *= TRUtils::GetFallOffMultOfDist(HitboxOwner->GetDistanceTo(GunOwner), GunOwner->GetStat_DmgDistFallOffMult(Shooter), GunConst::GUN_MAX_FALLOFF_DIST, GunConst::GUN_MIN_FALLOFF_DIST);
	}

	UGameplayStatics::ApplyPointDamage(HitboxOwner, Damage, NormalImpulse, Hit, Shooter->Controller, Shooter, GunOwner->GetStat_DamageType(Shooter));
}

void UWeaponFireComponent::OnPerHitscanObjectCollision(TWeakObjectPtr<UPrimitiveComponent> ObjectComp, FVector NormalImpulse, const FHitResult& Hit, AGunItem* GunOwner, AGameCharacter* Shooter, bool bIgnoreColVFX)
{
	if (!ObjectComp.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("OnHitscanObjectCollision - Invalid component!"));
		return;
	}
	if (!GunOwner || !Shooter)
	{
		UE_LOG(LogTemp, Error, TEXT("OnHitscanObjectCollision - invalid argument!"));
		return;
	}

	// 물리 충돌
	if (GunOwner && GunOwner->GetStat_ApplyImpactOnHit(Shooter) && Hit.Component->IsSimulatingPhysics())
	{
		Hit.Component->AddImpulseAtLocation(-Hit.ImpactNormal * GunOwner->GetStat_MissileMass(Shooter), Hit.ImpactPoint);
	}

	// VFX
	if (!bIgnoreColVFX && FireBulletStruct.HitObjFx)
	{
		Server_SpawnBulletHitObjectVFX(Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
}

void UWeaponFireComponent::OnHitscanFired(AGunItem* GunOwner, AGameCharacter* Shooter, FHitResult* FirstValidHit, int32 CharactersHit)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("OnHitscanFired - Invalid world!"));
		return;
	}
	if (!GunOwner || !Shooter)
	{
		UE_LOG(LogTemp, Error, TEXT("OnHitscanFired - invalid argument!"));
		return;
	}

	// 히트처리가 되었다면 최초 유효 충돌 정보를 기반으로 폭발 생성
	// 즉 관통 시에도 여러 번 폭발하지 않음
	if (FirstValidHit && GunOwner->GetStat_ExplodeOnHit(Shooter))
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (TRGM)
		{
			FActorSpawnParameters ExplParams;
			ExplParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ExplParams.Instigator = Shooter->GetInstigator();

			ATRExplosion* ExplSpawned = TRGM->SpawnExplosion(
				nullptr/* 총기 스텟을 기반으로 폭발을 동적으로 생성하기 때문에 별도의 클래스를 전달하지 않음*/,
				World,
				FirstValidHit->Location,
				FirstValidHit->ImpactNormal.Rotation(),
				FirstValidHit->Normal,
				ExplParams,
				GunOwner->GetStat_GunExplosionInfo(Shooter),
				true
			);
		}
	}

	// 격발자의 히트/미스 로직 처리
	if (CharactersHit > 0)
	{
		UGameplayStatics::ApplyDamage(
			Shooter,
			GunOwner->GetStat_SelfDmgOnCharacterHit(Shooter) * CharactersHit, // 캐릭터 1개체 당 데미지 반영
			Shooter->Controller,
			Shooter,
			UDamageTypeNeutral::StaticClass()
		);
	}
	else
	{
		UGameplayStatics::ApplyDamage(
			Shooter,
			GunOwner->GetStat_SelfDmgOnCharacterMissedHitscan(Shooter),
			Shooter->Controller,
			Shooter,
			UDamageTypeNeutral::StaticClass()
		);
	}
}

void UWeaponFireComponent::Local_Fx()
{
	if (!Local_CurrFxActor) return;

	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	if (!GunOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponFireComponent::Local_Fx - Invalid Actor Item!"));
		return;
	}

	// 로컬에서 fx를 처리하기 전에, 일단 현재 발사가 가능한지 확인
	// 확인 결과는 오직 FX 혹은 UI에만 반영되므로 치팅 소지가 없음
	if (Local_CurrFxActor->IsLocallyControlled())
	{
		// NOTE: 연사 도중 FX가 중지될 수 있기 때문에 Local_Fx에서 매 격발마다 확인
		Host_SetupFireFromMuzzle(Local_CurrFxActor); // 클라에서도 일부 FX처리를 위해서는 로직 값을 계산해야함
		if (!GunOwner->Host_CanFireShot())
		{
			Local_EndFx();
			return;
		}

		// 로컬에서 발사가 가능하다고 판단했을 경우 UI값을 추측해 사용; 이 값은 이후 동기화됨
		if (!Local_CurrFxActor->HasAuthority() && Local_CurrFxActor->Local_GetBoundHUDWidget().IsValid())
		{
			GunOwner->Client_PredictedCurrAmmo = GunOwner->CurrAmmo; // NOTE: OnRep은 최초 초기값 전달 시에는 호출되지 않기 때문에 여기서 한번 더 동기화를 시켜야 함
			GunOwner->Client_PredictedCurrAmmo = FMath::Max(0, GunOwner->Client_PredictedCurrAmmo - GunOwner->Client_PredictedAmmoPerShot);
			Local_CurrFxActor->Local_GetBoundHUDWidget()->UpdateAmmo(GunOwner->Client_PredictedCurrAmmo);
		}
	}

	// 반동
	if (Local_CurrFxActor->RecoilComponent)
	{
		EFireMode_PRAS CompFireMode = EFireMode_PRAS::Semi;
		switch (CurrFireMode)
		{
		case EWeaponFireMode::WFM_AUTO:
			CompFireMode = EFireMode_PRAS::Auto;
			break;
		case EWeaponFireMode::WFM_BURST:
			CompFireMode = EFireMode_PRAS::Burst; // TODO
			break;
		default:
		case EWeaponFireMode::WFM_SAFE: // 발생해선 안됨
		case EWeaponFireMode::WFM_SINGLE:
			CompFireMode = EFireMode_PRAS::Semi;
			break;
		}
		Local_CurrFxActor->RecoilComponent->SetFireMode(CompFireMode);
		Local_CurrFxActor->RecoilComponent->Play();
	}

	// 플레이어의 경우 카메라 셰이크
	AFPSCharacter* FPSPlayer = Cast<AFPSCharacter>(Local_CurrFxActor);
	if (FireCameraShakeClass && FPSPlayer && FPSPlayer->IsLocallyViewed() /* NOTE: PlayFx()는 멀티캐스트로 모든 호스트들에게 전달되지만 카메라 셰이크는 오직 로컬에서만 처리 */)
	{
		FPSPlayer->Local_PlayCameraShake(FireCameraShakeClass, 1.0f/*TODO: 총별로 반동 비례하게 세기 조정 + 필요 시 특수한 카메라 셰이크 추가*/);
	}

	// VFX
	if (GunOwner)
	{
		switch (CurrGunType)
		{
		case EWeaponFireType::WFT_PROJECTILE:
		{
			if (bUseMuzzleFlash_Projectile)
			{
				GunOwner->SpawnMuzzleFlashVFX();

				// 라이트
				if (GunOwner->bApplyLightToMuzzleOnFire)
				{
					GunOwner->FlashMuzzleLight(0.1f);
				}
			}

			// 탄피 배출
			if (bUseShellEjection_Projectile)
			{
				GunOwner->SpawnShellEjectVFX();
			}

			// 사운드
			if (bUseGunShotAudio_Projectile)
			{
				// NOTE: 각 호스트들이 듣는 총성은 서로 다를 수 있다
				GunOwner->Local_PlayGunshotSFX(GunOwner->Host_GetCurrAmmo() <= 0, FMath::FRandRange(0.5f, 0.6f), FMath::FRandRange(0.85f, 1.15f));
			}

			// TODO: 나머지 FX들 및 사운드 등
			break;
		}
		case EWeaponFireType::WFT_HITSCAN:
		{
			if (bUseMuzzleFlash_Hitscan)
			{
				GunOwner->SpawnMuzzleFlashVFX();

				// 라이트
				if (GunOwner->bApplyLightToMuzzleOnFire)
				{
					GunOwner->FlashMuzzleLight(0.1f);
				}
			}

			// 탄피 배출
			if (bUseShellEjection_Hitscan)
			{
				GunOwner->SpawnShellEjectVFX();
			}

			// 사운드
			if (bUseGunShotAudio_Hitscan)
			{
				// NOTE: 각 호스트들이 듣는 총성은 서로 다를 수 있다
				GunOwner->Local_PlayGunshotSFX(GunOwner->Host_GetCurrAmmo() <= 0, FMath::FRandRange(0.5f, 0.6f), FMath::FRandRange(0.15f, 1.85f));
			}

			// 트레이서 VFX
			// NOTE: 카메라 셰이크와 유사하게 로컬일 경우에만 처리한다; 로컬이 아닐 경우 멀티캐스트로 생성된다
			// NOTE: 중요 - 불릿 트레이서의 경우 FX들중 예외적으로, 별도의 멀티캐스트를 통해 재생한다
			// 로컬이 아닌 폰에 대한 FX를 처리할때 일반적으로 Multicast_PlayItemFxRPC를 통해 Local_Fx에서 처리되는데,
			// 불릿 트레이서의 경우 direction 정보를 클라이언트가 알 수가 없다
			// direction은 머즐 정보에 dependent하고 머즐은 카메라, 카메라는 컨트롤러 회전에 dependent하기 때문인데, 즉 다른 플레이어 컨트롤러 회전에 대한 정보가 없기 때문에
			// 클라는 다른 호스트 폰들의 트레이서 vfx를 올바르게 재생할 수 없다
			// 따라서 Multicast_PlayItemFxRPC 대신, direction 벡터를 함께 전송하는 별도의 RPC를 통해 처리한다
			if (Local_CurrFxActor->IsLocallyControlled() && GunOwner && FireBulletStruct.TracerFx)
			{
				for (int32 FireCnt = 1; FireCnt <= FireMissilePerShot; ++FireCnt)
				{
					// 탄퍼짐 계산
					Local_SpawnBulletTracerVFX(FixedRecoil(CurrCamMuzzleRotation.Vector(), CurrRecoilOffsetRange, FireCnt));
				}
			}

			break;
		}
		default:
			UE_LOG(LogTemp, Warning, TEXT("UWeaponFireComponent::Local_Fx - Unknown fire type!"));
		}
	}

	// TODO: SFX
}

void UWeaponFireComponent::Host_SetupFireFromMuzzle(AGameCharacter* FireActor)
{
	if (!FireActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Host_SetupFireFromMuzzle - Current actor is null!"));
		return;
	}
	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	if (!GunOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("Host_SetupFireFromMuzzle - Current owner is null!"));
		return;
	}

	// 액션에 필요한 정보 수집 및 등록
	TPair<FVector, FRotator> MuzzleInfo = FireActor->GetMuzzleInfo();
	CurrCamMuzzleLocation = MuzzleInfo.Get<0>();
	CurrCamMuzzleRotation = MuzzleInfo.Get<1>();

	FVector CurrCamMuzzleRotationVector = CurrCamMuzzleRotation.Vector();
	SetupVirtualTargetDistance(CurrCamMuzzleLocation, CurrCamMuzzleRotationVector, FireActor); // CurrFireActor는 순서 상 아직 설정되지 않았을 수 있다
	FVector VirtualTargetLocation = CurrCamMuzzleLocation + (CurrCamMuzzleRotationVector * CurrVirtualTargetDistance);

	UGunPartComponent* BarrelComp = GunOwner->GetBarrel();
	if (BarrelComp && BarrelComp->GetMeshComp())
	{
		CurrMeshMuzzleLocation = BarrelComp->GetMeshComp()->GetSocketLocation(MUZZLE_SOCKET);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Host_SetupFireFromMuzzle - Barrel component is null! Using root position as MeshMuzzleLocation."));
		CurrMeshMuzzleLocation = GunOwner->GetActorLocation();
	}
	CurrMeshMuzzleRotation = (VirtualTargetLocation - CurrMeshMuzzleLocation).Rotation();
}

FVector UWeaponFireComponent::RandomRecoil(FVector Direction, float OffsetRange)
{
	if (OffsetRange <= 0.0f) return Direction;
	FVector XVector = Direction.Cross(FVector::UpVector);
	if (XVector.IsNearlyZero())
	{
		XVector = Direction.Cross(FVector::ForwardVector);
	}
	FVector ZVector = Direction.Cross(XVector);
	FVector2D BulletSpread = FMath::RandPointInCircle(OffsetRange);
	return (Direction / Direction.Length() * 100) + (XVector.GetSafeNormal() * BulletSpread.X) + (ZVector.GetSafeNormal() * BulletSpread.Y);
}

FVector UWeaponFireComponent::FixedRecoil(FVector Direction, float OffsetRange, int32 ShotCount)
{
	float UseOffset = 0.0f;
	if (FireMissilePerShot > 1 && ShotCount == 1 && bFireSetAccurateOffsetForFirstMissile) UseOffset = 0.0f;
	else UseOffset = OffsetRange;

	FVector XVector = Direction.Cross(FVector::UpVector);
	if (XVector.IsNearlyZero())
	{
		XVector = Direction.Cross(FVector::ForwardVector);
	}
	FVector ZVector = Direction.Cross(XVector);

	FVector2D BulletSpread;
	if (ShotCount <= 9)
	{
		// 사각형 탄퍼짐인 점을 감안해 루트2로 나눈다
		BulletSpread = FixedPointInArea(OffsetRange / UE_SQRT_2, ShotCount - 1);
	}
	else
	{
		BulletSpread = RandPointInCircleSeeded(UseOffset, (ShotCount << 3) + 1 /* 수식은 큰 의미 없음 */);
	}
	return (Direction / Direction.Length() * 100) + (XVector.GetSafeNormal() * BulletSpread.X) + (ZVector.GetSafeNormal() * BulletSpread.Y);
}

FVector2D UWeaponFireComponent::FixedPointInArea(float SquareLen, int32 PointIdx)
{
	/* 5 3 6
	*  1 0 2
	*  7 4 8 
	*/
	TArray<FVector2D> BulletSpreads = { {0,0}, {-1,0}, {1,0}, {0,1}, {0,-1}, {-1,1}, {1,1}, {-1,-1}, {1,-1} };
	FVector2D Result = BulletSpreads[PointIdx];
	Result.X *= SquareLen;
	Result.Y *= SquareLen;
	return Result;
}

FVector2D UWeaponFireComponent::RandPointInCircleSeeded(float CircleRadius, int32 Seed)
{
	FMath::SRandInit(Seed);
	FVector2D Point;
	FVector2D::FReal L;
	do
	{
		Point.X = FMath::SRand() * 2.f - 1.f;
		Point.Y = FMath::SRand() * 2.f - 1.f;
		L = Point.SizeSquared();
	} while (L > 1.0f);

	return Point * CircleRadius;
}

void UWeaponFireComponent::OnSafeFire()
{
	return;
}

void UWeaponFireComponent::OnSingleFire()
{
	if (CurrFireActor)
	{
		Fire();
	}
	return;
}

void UWeaponFireComponent::OnAutoFire()
{
	// 이미 연사/점사 격발중일 경우 로직을 처리하지 않는다
	// 동일 프레임 내에 여러 인풋이 가해지는 엣지케이스를 방지
	if (CurrFireActor && !bIsRapidFiring)
	{
		// 초탄 즉각 발사
		Fire();

		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(RapidFireTimerHandle, RapidFireDelegate, CurrRapidFireInterval, true);
			bIsRapidFiring = true;
		}
	}
}

void UWeaponFireComponent::OnBurstFire()
{
	// 이미 연사/점사 격발중일 경우 로직을 처리하지 않는다
	// 동일 프레임 내에 여러 인풋이 가해지는 엣지케이스를 방지
	if (CurrFireActor && !bIsRapidFiring)
	{
		// TODO
	}
}

void UWeaponFireComponent::OnStopFire()
{
	if (CurrFireActor)
	{
		// FX 중단 멀티캐스팅
		CurrFireActor->Multicast_StopItemFxRPC(IsComponentPrimary(), Cast<APlayerController>(CurrFireActor->GetController()));
	}
	return;
}

void UWeaponFireComponent::OnStopRapidFire()
{
	bIsRapidFiring = false;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(RapidFireTimerHandle);
	}
	return;
}


void UWeaponFireComponent::PerformFire(AGameCharacter* Pawn, EWeaponFireMode FireMode)
{
	CurrFireActor = Pawn;
	switch (FireMode)
	{
	case EWeaponFireMode::WFM_SAFE:
		OnSafeFire();
		break;
	case EWeaponFireMode::WFM_SINGLE:
		OnSingleFire();
		break;
	case EWeaponFireMode::WFM_BURST:
		OnBurstFire();
		break;
	case EWeaponFireMode::WFM_AUTO:
		OnAutoFire();
		break;
	}

	if (CurrFireActor)
	{
		CurrFireActor->Multicast_PlayItemFxRPC(IsComponentPrimary(), Cast<APlayerController>(CurrFireActor->GetController()));
	}
}

void UWeaponFireComponent::StopFire(AGameCharacter* Pawn, EWeaponFireMode FireMode)
{
	OnStopFire();
	if (bIsRapidFiring)
	{
		OnStopRapidFire();
	}
	CurrFireActor = nullptr;
}

void UWeaponFireComponent::Local_OnSafeFx()
{
	return;
}

void UWeaponFireComponent::Local_OnSingleFx()
{
	if (Local_CurrFxActor)
	{
		Local_Fx();
	}
	return;
}

void UWeaponFireComponent::Local_OnAutoFx()
{
	if (Local_CurrFxActor && !bLocal_IsRapidFxPlaying)
	{
		// 첫 FX 즉각 처리
		Local_Fx();

		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(Local_RapidFxTimerHandle, Local_RapidFxDelegate, CurrRapidFireInterval/* 총기 발사 주기와 동일 */, true);
			bLocal_IsRapidFxPlaying = true;
		}
	}
}

void UWeaponFireComponent::Local_OnBurstFx()
{
	if (Local_CurrFxActor && !bLocal_IsRapidFxPlaying)
	{
		// TODO
	}
}

void UWeaponFireComponent::Local_OnStopFx()
{
	if (!Local_CurrFxActor) return;

	// 반동 중지
	if (Local_CurrFxActor->RecoilComponent)
	{
		Local_CurrFxActor->RecoilComponent->Stop();
	}

	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	if (!GunOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponFireComponent::Local_OnStopFx - Invalid Actor Item!"));
		return;
	}
	
	// NOTE: Niagara의 경우 알아서 해제됨
	return;
}

void UWeaponFireComponent::Local_OnStopRapidFx()
{
	bLocal_IsRapidFxPlaying = false;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(Local_RapidFxTimerHandle);
	}
	return;
}

void UWeaponFireComponent::Local_StartFx(AGameCharacter* Pawn, EWeaponFireMode FireMode)
{
	Local_CurrFxActor = Pawn;
	switch (FireMode)
	{
	case EWeaponFireMode::WFM_SAFE:
		Local_OnSafeFx();
		break;
	case EWeaponFireMode::WFM_SINGLE:
		Local_OnSingleFx();
		break;
	case EWeaponFireMode::WFM_BURST:
		Local_OnBurstFx();
		break;
	case EWeaponFireMode::WFM_AUTO:
		Local_OnAutoFx();
		break;
	}
}

void UWeaponFireComponent::Local_EndFx()
{
	Local_OnStopFx();
	if (bLocal_IsRapidFxPlaying)
	{
		Local_OnStopRapidFx();
	}
	Local_CurrFxActor = nullptr;
}

bool UWeaponFireComponent::TriggeredByPlayer(AFPSCharacter* PlayerPawn)
{
	if (!Super::TriggeredByPlayer(PlayerPawn)) return false;
	PerformFire(PlayerPawn, CurrFireMode);
	return true;
}

bool UWeaponFireComponent::TriggeredByAI(AGameCharacter* AIPawn)
{
	if (!Super::TriggeredByAI(AIPawn)) return false;
	PerformFire(AIPawn, CurrFireMode);
	return true;
}

bool UWeaponFireComponent::StoppedByPlayer(AFPSCharacter* PlayerPawn)
{
	if (!Super::StoppedByPlayer(PlayerPawn)) return false;
	StopFire(PlayerPawn, CurrFireMode);
	return true;
}

bool UWeaponFireComponent::StoppedByAI(AGameCharacter* AIPawn)
{
	Super::StoppedByAI(AIPawn);
	StopFire(AIPawn, CurrFireMode);
	return true;
}

void UWeaponFireComponent::Local_PlayFx(AGameCharacter* Invoker)
{
	Local_StartFx(Invoker, CurrFireMode);
}

void UWeaponFireComponent::Local_StopFx(AGameCharacter* Invoker)
{
	Local_EndFx();
}

void UWeaponFireComponent::Server_SpawnBulletHitObjectVFX(FVector HitLocation, FRotator HitRotation)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Multicast_SpawnBulletHitObjectVFX(HitLocation, HitRotation);
	}
}

void UWeaponFireComponent::Server_SpawnBulletHitCharacterVFX(FVector HitLocation, FRotator HitRotation)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Multicast_SpawnBulletHitCharacterVFX(HitLocation, HitRotation);
	}
}

void UWeaponFireComponent::Multicast_SpawnBulletTracerVFX_Implementation(FVector Direction, APlayerController* InvokeHost)
{
	if (InvokeHost && InvokeHost->IsLocalPlayerController())
	{
		// 최초 재생 주체는 이미 FX를 처리했으므로 무시한다
		return;
	}
	Local_SpawnBulletTracerVFX(Direction);
}

void UWeaponFireComponent::Local_SpawnBulletTracerVFX(FVector Direction)
{
	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	ATRPlayerController* TRPC = GetWorld()->GetFirstPlayerController<ATRPlayerController>();
	if (GunOwner && TRPC && FireBulletStruct.TracerFx)
	{
		TPair<USceneComponent*, FName> MuzzleInfo = GunOwner->GetMuzzleCompAndSock();
		if (MuzzleInfo.Get<0>())
		{
			UNiagaraComponent* Spawned = TRUtils::TRSpawnSystemAtLocation(
				TRPC->TracerVFXPendingRelease,
				TRPC->CurrTracerVFXPendingReleaseCnt,
				TRPC->MaxCoexistingTracerVFXCnt,
				GetWorld(),
				FireBulletStruct.TracerFx,
				MuzzleInfo.Get<0>()->GetComponentLocation(),
				Direction.Rotation(),
				FVector(1.0f),
				true,
				true
			);

			TRUtils::OptimizePrimitiveComp(Spawned);
		}
	}
}

void UWeaponFireComponent::Multicast_SpawnBulletHitObjectVFX_Implementation(FVector HitLocation, FRotator HitRotation)
{
	ATRPlayerController* TRPC = GetWorld()->GetFirstPlayerController<ATRPlayerController>();
	if (TRPC && FireBulletStruct.HitObjFx)
	{
		UNiagaraComponent* Spawned = TRUtils::TRSpawnSystemAtLocation(
			TRPC->HitVFXPendingRelease,
			TRPC->CurrHitVFXPendingReleaseCnt,
			TRPC->MaxCoexistingHitVFXCnt,
			GetWorld(),
			FireBulletStruct.HitObjFx,
			HitLocation,
			HitRotation,
			FVector(1.0f),
			true,
			true
		);

		TRUtils::OptimizePrimitiveComp(Spawned);
	}
}

void UWeaponFireComponent::Multicast_SpawnBulletHitCharacterVFX_Implementation(FVector HitLocation, FRotator HitRotation)
{
	ATRPlayerController* TRPC = GetWorld()->GetFirstPlayerController<ATRPlayerController>();
	if (TRPC && FireBulletStruct.HitCharFx)
	{
		UNiagaraComponent* Spawned = TRUtils::TRSpawnSystemAtLocation(
			TRPC->HitVFXPendingRelease,
			TRPC->CurrHitVFXPendingReleaseCnt,
			TRPC->MaxCoexistingHitVFXCnt,
			GetWorld(),
			FireBulletStruct.HitCharFx,
			HitLocation,
			HitRotation,
			FVector(1.0f),
			true,
			true
		);

		TRUtils::OptimizePrimitiveComp(Spawned);
	}
}

void UWeaponFireComponent::FireHitscan()
{
	if (!CurrFireActor)
	{
		UE_LOG(LogTemp, Error, TEXT("FireHitscan - Fire actor is null!"));
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FireHitscan - World is null!"));
		return;
	}

	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	if (!GunOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("FireHitscan - GunOwner is null!"));
		return;
	}

	// 캐싱
	for (int32 FireCnt = 1; FireCnt <= FireMissilePerShot; ++FireCnt)
	{
		// 탄퍼짐 계산
		CurrMeshMuzzleRotation = FixedRecoil(CurrCamMuzzleRotation.Vector(), CurrRecoilOffsetRange, FireCnt).Rotation();

		// 히트스캔 LineTrace는 메쉬 머즐을 시작점으로 처리한다
		FVector FireDirection = CurrMeshMuzzleRotation.Vector();
		FVector LineTraceEndLocation = CurrMeshMuzzleLocation + (FireDirection * MaxTargetRange);

		// 트레이서 VFX
		if (FireBulletStruct.TracerFx)
		{
			Multicast_SpawnBulletTracerVFX(FireDirection, Cast<APlayerController>(CurrFireActor->GetController()));
		}

		/*if (FireCnt == 1) DrawDebugLine(GetWorld(), CurrMeshMuzzleLocation, LineTraceEndLocation, FColor::Red, false, 10.0f);
		else DrawDebugLine(GetWorld(), CurrMeshMuzzleLocation, LineTraceEndLocation, FColor::Blue, false, 3.0f);*/

		FHitResult FirstValidHitResult;
		TArray<UPrimitiveComponent*> IgnoredComps;
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(GetOwner());
		IgnoredActors.Add(CurrFireActor);

		int32 CharactersHit = 0;
		bool bIsHitResValid = false;
		bool bIgnoreColVFX = GunOwner->GetStat_ExplodeOnHit(CurrFireActor); // 폭발 발생 시 충돌 VFX 재생을 생략함
		bool bHitProcessed = ProcessHitscanSingle(
			FirstValidHitResult,
			bIsHitResValid,
			CharactersHit,
			CurrMeshMuzzleLocation,
			LineTraceEndLocation,
			FireDirection,
			0,
			TR_DEFAULT_HITSCAN_RECURSION,
			IgnoredComps,
			IgnoredActors,
			bIgnoreColVFX
		);

		if (bHitProcessed)
		{
			OnHitscanFired(
				GunOwner,
				CurrFireActor,
				bIsHitResValid ? &FirstValidHitResult : nullptr,
				CharactersHit
			);
		}
	}
}

void UWeaponFireComponent::FireProjectile()
{
	if (!CurrFireActor)
	{
		UE_LOG(LogTemp, Error, TEXT("FireProjectile - Fire actor is null!"));
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FireProjectile - World is null!"));
		return;
	}
	if (!FireProjClass)
	{
		UE_LOG(LogTemp, Error, TEXT("FireProjectile - Projectile class not bound - %s!"), *(this->GetName()));
		return;
	}

	// 발사 FX
	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());

	// 캐싱
	for (int32 FireCnt = 1; FireCnt <= FireMissilePerShot; ++FireCnt)
	{
		// 탄퍼짐 계산
		CurrMeshMuzzleRotation = FixedRecoil(CurrCamMuzzleRotation.Vector(), CurrRecoilOffsetRange, FireCnt).Rotation();

		// 투사체 발사
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = CurrFireActor;
		SpawnParams.Instigator = CurrFireActor->GetInstigator();

		// 투사체는 메쉬 위치를 기준으로 발사한다
		ABaseProjectile* SpawnProjectile = World->SpawnActor<ABaseProjectile>(
			FireProjClass,
			CurrMeshMuzzleLocation,
			CurrMeshMuzzleRotation, // 초기 발사 방향
			SpawnParams
		);

		if (SpawnProjectile)
		{
			// 설정 초기화
			// NOTE: Projectile의 damage type를 오버라이드
			SpawnProjectile->DamageType = CurrDamageType;

			// 총기에서 발사된 경우 추가 초기화
			if (GunOwner)
			{
				SpawnProjectile->EnemyDamage = GunOwner->GetStat_DmgEnemyDirect(CurrFireActor);
				SpawnProjectile->AllyDamage = GunOwner->GetStat_DmgAllyDirect(CurrFireActor);
				SpawnProjectile->SelfDmgOnCharacterHit = GunOwner->GetStat_SelfDmgOnCharacterHit(CurrFireActor);
				SpawnProjectile->DmgMultOnAirshot = GunOwner->GetStat_DmgMultOnAirshot(CurrFireActor);
				SpawnProjectile->HeadshotMultiplier = GunOwner->GetStat_DmgMultOnHead(CurrFireActor);
				SpawnProjectile->bShouldAffectForceOnCollision = GunOwner->GetStat_ApplyImpactOnHit(CurrFireActor);
				SpawnProjectile->ImpactMass = GunOwner->GetStat_MissileMass(CurrFireActor);
				SpawnProjectile->DestroyOnHitCount = GunOwner->GetStat_ProjDestroyOnHitCount(CurrFireActor);
				SpawnProjectile->bPiercePawns = GunOwner->GetStat_ProjPiercePawns(CurrFireActor);
				SpawnProjectile->bHasDmgDistFallOff = GunOwner->GetStat_HasDmgDistFallOff(CurrFireActor);
				SpawnProjectile->DmgDistFallOffMult = GunOwner->GetStat_DmgDistFallOffMult(CurrFireActor);
				SpawnProjectile->DmgMultDistClose = GunOwner->GetStat_DmgMultDistClose(CurrFireActor);
				SpawnProjectile->DmgMultDistFar = GunOwner->GetStat_DmgMultDistFar(CurrFireActor);

				SpawnProjectile->bShouldSpawnExplOnCollision = GunOwner->GetStat_ExplodeOnHit(CurrFireActor);
				if (SpawnProjectile->bShouldSpawnExplOnCollision)
				{
					SpawnProjectile->bShouldOverrideExplosiveinfo = true;
					SpawnProjectile->NewExplosiveInfo = GunOwner->GetStat_GunExplosionInfo(CurrFireActor);
				}

				SpawnProjectile->bApplyStatEffToEnemyOnHit = GunOwner->GetStat_ApplyStatEffToEnemyOnHit(CurrFireActor);
				SpawnProjectile->StatEffsToEnemyWhenHit = GunOwner->GetStat_StatEffsToEnemyWhenHit(CurrFireActor);
				SpawnProjectile->bApplyStatEffToAllyOnHit = GunOwner->GetStat_ApplyStatEffToAllyOnHit(CurrFireActor);
				SpawnProjectile->StatEffsToAllyWhenHit = GunOwner->GetStat_StatEffsToAllyWhenHit(CurrFireActor);

				SpawnProjectile->bBounceOffObjIndefinitely = GunOwner->GetStat_ProjBounceOffObjIndefinitely(CurrFireActor);

				if (SpawnProjectile->ProjectileMovementComponent)
				{
					SpawnProjectile->ProjectileMovementComponent->InitialSpeed = GunOwner->GetStat_ProjInitialSpeed(CurrFireActor);
					SpawnProjectile->ProjectileMovementComponent->MaxSpeed = GunOwner->GetStat_ProjMaxSpeed(CurrFireActor);
					SpawnProjectile->ProjectileMovementComponent->bRotationFollowsVelocity = GunOwner->GetStat_ProjRotationFollowsVelocity(CurrFireActor);
					SpawnProjectile->ProjectileMovementComponent->bRotationRemainsVertical = GunOwner->GetStat_ProjRotationRemainsVertical(CurrFireActor);
					SpawnProjectile->ProjectileMovementComponent->bShouldBounce = GunOwner->GetStat_ProjShouldBounce(CurrFireActor);
					SpawnProjectile->ProjectileMovementComponent->Bounciness = GunOwner->GetStat_ProjBounciness(CurrFireActor);
					SpawnProjectile->ProjectileMovementComponent->ProjectileGravityScale = GunOwner->GetStat_ProjGravityScale(CurrFireActor);

					// 중요: 간소화된 TR용 투사체 파이프라인만으로는 바운스를 처리할 수 없다
					// 따라서 이 경우 기존 핸들러를 반드시 사용해야 한다
					if (SpawnProjectile->ProjectileMovementComponent->bShouldBounce)
					{
						SpawnProjectile->ProjectileMovementComponent->bUseDefaultHitHandlers = true;
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FireProjectile - Projectile instantiation failed! %s"), *(this->GetName()));
		}
	}
}

bool UWeaponFireComponent::ProcessHitscanSingle(FHitResult& out_HitResult, bool& out_bHitResultValid, int32& out_CharactersHit, const FVector& LineTraceBegin, const FVector& LineTraceEnd, const FVector& TraceDirection, uint8 RecursiveDepth, uint8 MaxRecursiveDepth, TArray<UPrimitiveComponent*>& IgnoredComponents, TArray<AActor*>& IgnoredActors, bool bIgnoreColVFX)
{
	if (RecursiveDepth > MaxRecursiveDepth) return false;
	if (RecursiveDepth > TR_MAX_HITSCAN_RECURSION)
	{
		UE_LOG(LogTemp, Fatal, TEXT("ProcessHitscanSingle - Risk of stack overflow! Check immediately."))
		return false; // Hard-limit
	}

	AGunItem* GunOwner = Cast<AGunItem>(GetOwner());
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActors(IgnoredActors);
	CollisionParams.AddIgnoredComponents(IgnoredComponents);

	// 최초 유효 충돌 대상 계산, 외곽 히트박스가 아닌 경우 즉각 처리
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByProfile(HitResult, LineTraceBegin, LineTraceEnd, TEXT("HitscanProfile"), CollisionParams))
	{
		UOuterHitboxComponent* OuterHitboxComp = Cast<UOuterHitboxComponent>(HitResult.Component);
		UHitboxComponent* HitboxComp = Cast<UHitboxComponent>(HitResult.Component);
		if (OuterHitboxComp)
		{
			OuterHitboxComp->OnOuterHitboxCollision(0.0f/*1틱*/);
			IgnoredComponents.Add(Cast<UPrimitiveComponent>(OuterHitboxComp));

			// 이번 호출에서 검출한 외곽 히트박스를 제외하고 다시 히트 계산을 재귀적으로 처리
			return ProcessHitscanSingle(out_HitResult, out_bHitResultValid, out_CharactersHit, LineTraceBegin, LineTraceEnd, TraceDirection, RecursiveDepth + 1, MaxRecursiveDepth, IgnoredComponents, IgnoredActors, bIgnoreColVFX);
		}
		else if (HitboxComp)
		{
			if (!out_bHitResultValid)
			{
				out_HitResult = HitResult;
				out_bHitResultValid = true;
			}

			OnPerHitscanHitboxCollision(HitboxComp, HitResult.ImpactNormal, HitResult, GunOwner, CurrFireActor, bIgnoreColVFX);
			IgnoredActors.Add(HitResult.GetActor());

			out_CharactersHit++;

			if (GunOwner->GetStat_HitscanPiercePawns(CurrFireActor))
			{
				// 폰 관통이 가능한 경우에 한해 이번 호출에서 검출한 세부 히트박스의 오너를 제외하고 다시 히트 계산을 재귀적으로 처리
				// 같은 액터에 대해 여러 번 히트를 처리하는 일을 방지
				// 관통이 불가능한 경우 처음 처리되는 히트박스 하나에 대해서만 연산을 진행
				return ProcessHitscanSingle(out_HitResult, out_bHitResultValid, out_CharactersHit, LineTraceBegin, LineTraceEnd, TraceDirection, RecursiveDepth + 1, MaxRecursiveDepth, IgnoredComponents, IgnoredActors, bIgnoreColVFX);
			}
		}
		else
		{
			if (!out_bHitResultValid)
			{
				out_HitResult = HitResult;
				out_bHitResultValid = true;
			}

			OnPerHitscanObjectCollision(HitResult.Component, HitResult.ImpactNormal, HitResult, GunOwner, CurrFireActor, bIgnoreColVFX);
			IgnoredActors.Add(HitResult.GetActor());

			// 현재 오브젝트 관통은 불가하므로 재귀 없음
		}
	}
	return true;
}
