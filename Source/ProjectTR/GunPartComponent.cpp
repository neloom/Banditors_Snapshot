// Copyright (C) 2024 by Haguk Kim


#include "GunPartComponent.h"
#include "GunItem.h"
#include "BaseProjectile.h"
#include "BulletTraceProjectile.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

UGunPartComponent::UGunPartComponent()
{
	// 개별 파츠는 별도의 틱을 사용하지 않는다
	PrimaryComponentTick.bCanEverTick = false;

	// 레플리케이션 활성화
	SetIsReplicatedByDefault(true);
}

void UGunPartComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGunPartComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGunPartComponent::SetupMeshComp(UStaticMesh* Static, USkeletalMesh* Skeletal)
{
	if (!IsValid(Static) && !IsValid(Skeletal))
	{
		UE_LOG(LogTemp, Error, TEXT("UGunPartComponent - %s has neither static nor skeletal mesh component set."), *GetName());
		return;
	}
	if (IsValid(Static) && IsValid(Skeletal))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGunPartComponent - %s has both static and skeletal mesh component set as non-null. Prioritizing static mesh component."), *GetName());
	}

	// 기존 MeshComponent가 있을 경우 탈착 후 해제한다
	// 일반적인 상황에서는 기존 MeshComponent가 존재해서는 안된다
	if (MeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UGunPartComponent - MeshComponent already exists for %s! This is an unintended behaviour."), *GetName());
		MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		MeshComponent->DestroyComponent();
		MeshComponent = nullptr;
	}

	if (IsValid(Static))
	{
		MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunPartMeshComp_Static"));
		UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(MeshComponent);
		if (StaticMeshComp)
		{
			StaticMesh = Static;
			StaticMeshComp->SetStaticMesh(StaticMesh);
		}
	}
	else
	{
		MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunPartMeshComp_Skeletal"));
		USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(MeshComponent);
		if (SkeletalMeshComp)
		{
			SkeletalMesh = Skeletal;
			SkeletalMeshComp->SetSkeletalMesh(Skeletal);
		}
	}
	// 중요:
	// Mesh component의 아우터를 이 컴포넌트가 아닌 이 컴포넌트의 액터로 설정한다
	// Mesh component의 경우 Gun generation 과정에서 아이템의 루트가 될 수 있기 때문에 outer가 이 컴포넌트가 아닌 이 컴포넌트의 Outer(액터)가 되어야 한다
	MeshComponent->Rename(nullptr, GetOuter());

	// 중요:
	// Mesh component의 경우 NewObject로 동적으로 생성할 경우 RegisterComponent()를 수동으로 호출해주어야 RenderState가 생성되어 렌더링이 처리됨
	if (GetWorld()) MeshComponent->RegisterComponent();
	return;
}

UMeshComponent* UGunPartComponent::GetMeshComp()
{
	return MeshComponent;
}

EGunPartMeshType UGunPartComponent::GetMeshCompType()
{
	if (!IsValid(MeshComponent)) return EGunPartMeshType::EGM_Invalid;
	if (IsValid(Cast<UStaticMeshComponent>(MeshComponent))) return EGunPartMeshType::EGM_Static;
	else if (IsValid(Cast<USkeletalMeshComponent>(MeshComponent))) return EGunPartMeshType::EGM_Skeletal;
	return EGunPartMeshType::EGM_Invalid;
}

const FVector& UGunPartComponent::GetMeshOffset()
{
	return MeshOffset;
}

void UGunPartComponent::StatPass1(AGunItem* Gun)
{
	/* 적중 */
	Gun->SetStat_DmgEnemyDirect(Gun->GetStat_DmgEnemyDirect(nullptr) + DeltaDmgEnemyDirect);
	Gun->SetStat_DmgAllyDirect(Gun->GetStat_DmgAllyDirect(nullptr) + DeltaDmgAllyDirect);
	Gun->SetStat_SelfDmgOnCharacterHit(Gun->GetStat_SelfDmgOnCharacterHit(nullptr) + DeltaSelfDmgOnCharacterHit);
	Gun->SetStat_SelfDmgOnCharacterMissedHitscan(Gun->GetStat_SelfDmgOnCharacterMissedHitscan(nullptr) + DeltaSelfDmgOnCharacterMissedHitscan);
	Gun->SetStat_SelfDmgOnCharacterKilled(Gun->GetStat_SelfDmgOnCharacterKilled(nullptr) + DeltaSelfDmgOnCharacterKilled);

	/* 다중 발사 */
	Gun->SetStat_MissileSpawnedPerShot(Gun->GetStat_MissileSpawnedPerShot(nullptr) + DeltaMissileSpawnedPerShot);

	/* 거리 별 데미지 변화 */
	Gun->SetStat_DmgMultDistClose(Gun->GetStat_DmgMultDistClose(nullptr) + DeltaDmgMultDistClose);
	Gun->SetStat_DmgMultDistFar(Gun->GetStat_DmgMultDistFar(nullptr) + DeltaDmgMultDistFar);

	/* 에어샷 */
	Gun->SetStat_DmgMultOnAirshot(Gun->GetStat_DmgMultOnAirshot(nullptr) + DeltaDmgMultOnAirshot);

	/* 폭파 */
	Gun->SetStat_GunExplosionDamage(Gun->GetStat_GunExplosionDamage(nullptr) + DeltaExplosionDmg);
	Gun->SetStat_GunExplosionRadius(Gun->GetStat_GunExplosionRadius(nullptr) + DeltaExplosionRadius);
	Gun->SetStat_GunExplImpactStrength(Gun->GetStat_GunExplImpactStrength(nullptr) + DeltaBaseImpactStrength);
	Gun->SetStat_GunDmgMultOnExplInstigator(Gun->GetStat_GunDmgMultOnExplInstigator(nullptr) + DeltaDmgMultOnExplInstigator);

	/* 피격 지점 배율 */
	Gun->SetStat_DmgMultOnHead(Gun->GetStat_DmgMultOnHead(nullptr) + DeltaDmgMultOnHead);

	/* 기타 데미지 조정 */
	Gun->SetStat_DmgDistFallOffMult(Gun->GetStat_DmgDistFallOffMult(nullptr) + DeltaDmgDistFallOffMult);

	/* 상태이상 */
	Gun->AddStat_StatEffToEnemyWhenHit(AddStatEffsToEnemyWhenHit);
	Gun->AddStat_StatEffToAllyWhenHit(AddStatEffsToAllyWhenHit);

	/* 발사 속도 */
	Gun->SetStat_FireInterval(Gun->GetStat_FireInterval(nullptr) + DeltaFireInterval);

	/* 탄 퍼짐 */
	Gun->SetStat_RecoilOffsetRange(Gun->GetStat_RecoilOffsetRange(nullptr) + DeltaRecoilOffsetRange);

	/* 장탄 */
	Gun->SetStat_MaxAmmo(Gun->GetStat_MaxAmmo(nullptr) + DeltaMaxAmmo);
	Gun->SetStat_AmmoPerShot(Gun->GetStat_AmmoPerShot(nullptr) + DeltaAmmoPerShot);

	/* 투사체 속도 */
	Gun->SetStat_ProjInitialSpeed(Gun->GetStat_ProjInitialSpeed(nullptr) + DeltaProjInitialSpeed);
	Gun->SetStat_ProjMaxSpeed(Gun->GetStat_ProjMaxSpeed(nullptr) + DeltaProjMaxSpeed);
}

void UGunPartComponent::StatPass2(AGunItem* Gun)
{
	/* 적중 */
	Gun->SetStat_DmgEnemyDirect(Gun->GetStat_DmgEnemyDirect(nullptr) * MultDmgEnemyDirect);
	Gun->SetStat_DmgAllyDirect(Gun->GetStat_DmgAllyDirect(nullptr) * MultDmgAllyDirect);

	////////////////////////
}

void UGunPartComponent::StatPass3(AGunItem* Gun)
{
	/* 다중 발사 */
	if (bOverrideSetAccurateOffsetForFirstMissile)
	{
		Gun->SetStat_SetAccurateOffsetForFirstMissile(bSetAccurateOffsetForFirstMissileValue);
	}

	/* 불릿 트레이서 타입 */
	if (bSetBulletFromEnum && Gun->BulletConfig)
	{
		Gun->BulletStruct = Gun->BulletConfig->SearchBulletFromEnum(BulletEnum);
		Gun->bBulletStructValid = true; // nullcheck용 플래그
	}

	/* 머즐 플래시 VFX */
	if (bSetMuzzleFlashVFXFromEnum && Gun->FxConfig)
	{
		Gun->MuzzleFlashVFX = Gun->FxConfig->SearchNiagaraFromEnum(MuzzleFlashVFXEnum);
	}

	/* 머즐 라이팅 VFX */
	if (bOverrideApplyLightToMuzzleOnFire)
	{
		Gun->bApplyLightToMuzzleOnFire = bApplyLightToMuzzleOnFireValue;
	}
	if (bOverrideMuzzleLightColor)
	{
		Gun->MuzzleLightColor = MuzzleLightColorValue;
	}
	if (bOverrideMuzzleLightIntensity)
	{
		Gun->MuzzleLightIntensity = MuzzleLightIntensityValue;
	}
	
	/* 탄피 배출 VFX */
	if (bSetShellEjectVFXFromEnum && Gun->FxConfig)
	{
		Gun->ShellEjectVFX = Gun->FxConfig->SearchNiagaraFromEnum(ShellEjectVFXEnum);
	}

	/* 카메라 셰이크 */
	if (bSetCameraShakeFromEnum && Gun->CamShakeConfig)
	{
		Gun->CameraShakeClass = Gun->CamShakeConfig->SearchCameraShakeFromEnum(CameraShakeClassEnum);
	}

	/* 무기 타입 */
	if (bOverrideGunType)
	{
		Gun->SetStat_GunType(GunTypeValue);
	}
	if (bOverrideFireMode)
	{
		Gun->SetStat_FireMode(FireModeValue);
	}
	if (bOverrideDamageType)
	{
		Gun->SetStat_DamageType(DamageTypeValue);
	}

	/* 관통 */
	if (bOverrideHitscanPiercePawns)
	{
		Gun->SetStat_HitscanPiercePawns(bHitscanPiercePawnsValue);
	}

	/* 폭파 */
	if (bOverrideExplodeOnHit)
	{
		Gun->SetStat_ExplodeOnHit(bExplodeOnHitValue);
	}
	if (bOverrideMinExplosionMultiplier)
	{
		Gun->SetStat_GunMinExplosionMultiplier(MinExplosionMultiplierValue);
	}

	/* 폭파 VFX */
	if (bOverrideExplosionVFXEnum)
	{
		Gun->ExplosionVFXEnum = ExplosionVFXEnumValue;
	}
	if (bOverrideExplVFXRadiusConstant)
	{
		Gun->ExplosionVFXRadiusConstant = ExplVFXRadiusConstantValue;
	}

	/* 기타 데미지 조정 */
	if (bOverrideHasDmgDistFallOff)
	{
		Gun->SetStat_HasDmgDistFallOff(bHasDmgDistFallOffValue);
	}

	/* 상태이상 */
	if (bOverrideApplyStatEffToEnemyOnHit)
	{
		Gun->SetStat_ApplyStatEffToEnemyOnHit(bApplyStatEffToEnemyOnHitValue);
	}
	if (bOverridebApplyStatEffToAllyOnHit)
	{
		Gun->SetStat_ApplyStatEffToAllyOnHit(bApplyStatEffToAllyOnHitValue);
	}

	/* 충격량 배수 */
	if (bOverrideApplyImpactOnHit)
	{
		Gun->SetStat_ApplyImpactOnHit(bApplyImpactOnHitValue);
	}
	if (bOverrideMissileMass)
	{
		Gun->SetStat_MissileMass(MissileMassValue);
	}

	////////////////

	/* 투사체 종류 */
	if (bSetProjectileFromEnum && Gun->ProjectileConfig)
	{
		Gun->SetStat_ProjectileClass(Gun->ProjectileConfig->SearchProjectileFromEnum(ProjectileEnum));
	}

	/* 투사체 회전 */
	if (bOverrideProjRotationFollowsVelocity)
	{
		Gun->SetStat_ProjRotationFollowsVelocity(bProjRotationFollowsVelocityValue);
	}
	if (bOverrideProjRotationRemainsVertical)
	{
		Gun->SetStat_ProjRotationRemainsVertical(bProjRotationRemainsVerticalValue);
	}

	/* 투사체 바운스 */
	if (bOverrideProjShouldBounce)
	{
		Gun->SetStat_ProjShouldBounce(bProjShouldBounceValue);
	}
	if (bOverrideProjBounciness)
	{
		Gun->SetStat_ProjBounciness(ProjBouncinessValue);
	}
	if (bOverrideProjBounceOffObjIndefinitely)
	{
		Gun->SetStat_ProjBounceOffObjIndefinitely(bProjBounceOffObjIndefinitelyValue);
	}

	/* 투사체 파괴 조건 */
	if (bOverrideProjDestroyOnHitCount)
	{
		Gun->SetStat_ProjDestroyOnHitCount(ProjDestroyOnHitCountValue);
	}

	/* 투사체 관통여부 */
	if (bOverrideProjPiercePawns)
	{
		Gun->SetStat_ProjPiercePawns(bProjPiercePawnsValue);
	}

	/* 투사체 중력 */
	if (bOverrideProjGravityScale)
	{
		Gun->SetStat_ProjGravityScale(ProjGravityScaleValue);
	}

	////////////////

	/* 격발 SFX */
	if (bSetGunshotSFXFromEnum && Gun->AudioConfig)
	{
		Gun->Gunshot1SFX = Gun->AudioConfig->SearchAudioFromEnum(Gunshot1SFXEnum);
		Gun->Gunshot2SFX = Gun->AudioConfig->SearchAudioFromEnum(Gunshot2SFXEnum);
		Gun->Gunshot3SFX = Gun->AudioConfig->SearchAudioFromEnum(Gunshot3SFXEnum);
		Gun->Gunshot4SFX = Gun->AudioConfig->SearchAudioFromEnum(Gunshot4SFXEnum);
		Gun->Gunshot5SFX = Gun->AudioConfig->SearchAudioFromEnum(Gunshot5SFXEnum);
		Gun->Gunshot6SFX = Gun->AudioConfig->SearchAudioFromEnum(Gunshot6SFXEnum);
		Gun->Gunshot7SFX = Gun->AudioConfig->SearchAudioFromEnum(Gunshot7SFXEnum);
		Gun->Gunshot8SFX = Gun->AudioConfig->SearchAudioFromEnum(Gunshot8SFXEnum);
	}
	if (bSetOnEmptyFireSFXFromEnum && Gun->AudioConfig)
	{
		Gun->OnEmptyFireSFX = Gun->AudioConfig->SearchAudioFromEnum(OnEmptyFireSFXEnum);
	}

	////////////////

	// 정수 카운터 사용 여부
	if (bOverrideUseIntCounter)
	{
		Gun->bUseIntCounter = bUseIntCounterValue;
	}

	// 게이지 사용 여부
	if (bOverrideUseGauge)
	{
		Gun->bUseGauge = bUseGaugeValue;
	}
}

