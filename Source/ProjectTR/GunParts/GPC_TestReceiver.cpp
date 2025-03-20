// Copyright (C) 2024 by Haguk Kim


#include "GPC_TestReceiver.h"
#include "TRMacros.h"
#include "TREnums.h"
#include "DamageTypeElemental.h"

UGPC_TestReceiver::UGPC_TestReceiver()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_RIFLE_4));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_PROJECTILE;

	bOverrideFireMode = true;
	FireModeValue = EWeaponFireMode::WFM_AUTO;

	bOverrideHasDmgDistFallOff = false;
	bHasDmgDistFallOffValue = true;

	DeltaDmgDistFallOffMult = -0.3f;

	DeltaFireInterval = -0.95f;

	DeltaDmgEnemyDirect = 10.0f;
	DeltaDmgAllyDirect = 1.0f;
	DeltaDmgMultOnHead = 100.0f;

	bOverrideApplyImpactOnHit = true;
	bApplyImpactOnHitValue = true;

	DeltaProjInitialSpeed = 1000.0f;
	DeltaProjMaxSpeed = 1000.0f;
	
	bOverrideProjShouldBounce = true;
	bProjShouldBounceValue = true;

	bOverrideProjBounceOffObjIndefinitely = true;
	bProjBounceOffObjIndefinitelyValue = true;

	bOverrideProjDestroyOnHitCount = true;
	ProjDestroyOnHitCountValue = 1;

	bOverrideProjGravityScale = true;
	ProjGravityScaleValue = 0.5f;

	bOverrideProjBounciness = true;
	ProjBouncinessValue = 0.75f;

	bOverrideProjPiercePawns = true;
	bProjPiercePawnsValue = false;

	//DeltaSelfDmgOnCharacterHit = -1.0f;
	//DeltaSelfDmgOnCharacterMissedHitscan = 1.0f;

	DeltaDmgMultOnAirshot = 10.0f;

	DeltaSelfDmgOnCharacterKilled = -10.0f;

	//DeltaDmgMultDistClose = 20.0f;
	//DeltaDmgMultDistFar = 20.0f;

	///////////////// TESTING
	bSetCameraShakeFromEnum = true;
	CameraShakeClassEnum = ECamShakeReference::ECR_OnFireCamShake;
	bSetMuzzleFlashVFXFromEnum = true;
	MuzzleFlashVFXEnum = ENiagaraReference::ENR_MZF_Energy_1;
	bSetProjectileFromEnum = true;
	ProjectileEnum = EProjectileReference::EPR_DefaultProj;
	bSetShellEjectVFXFromEnum = true;
	ShellEjectVFXEnum = ENiagaraReference::ENR_SEJ_Default;

	bOverrideApplyLightToMuzzleOnFire = true;
	bApplyLightToMuzzleOnFireValue = true;
	bOverrideMuzzleLightColor = true;
	MuzzleLightColorValue = FColor(255, 0, 255);
	bOverrideMuzzleLightIntensity = true;
	MuzzleLightIntensityValue = 5000.0f;

	bOverrideHitscanPiercePawns = true;
	bHitscanPiercePawnsValue = true;


	//// 
	//DeltaMissileSpawnedPerShot = 10;
	bOverrideSetAccurateOffsetForFirstMissile = true;
	bSetAccurateOffsetForFirstMissileValue = true;
	//DeltaRecoilOffsetRange = 5.0f; // 산탄

	////////////
	//bOverrideExplodeOnHit = true;
	//bExplodeOnHitValue = true;
	DeltaExplosionDmg = 5.0f;
	DeltaExplosionRadius = 300.0f;
	DeltaDmgMultOnExplInstigator = 0.0f;
	bOverrideMinExplosionMultiplier = true;
	MinExplosionMultiplierValue = 1.0f;
	DeltaBaseImpactStrength = 1000.0f;
	bOverrideExplosionVFXEnum = true;
	ExplosionVFXEnumValue = ENiagaraReference::ENR_EXP_Default;
	bOverrideExplVFXRadiusConstant = true;
	ExplVFXRadiusConstantValue = 0.01f;

	bOverrideDamageType = true;
	DamageTypeValue = UDamageTypeElemental::StaticClass();

	FStatEffectGenInfo EffToEnemy;
	EffToEnemy.StatusEffectId = "TESTEFFECT";
	EffToEnemy.StatusName = "TESTNAME";
	EffToEnemy.StatModifier.DeltaMaxWalkSpeed = -1200.0f;
	EffToEnemy.StatDuration = 3.0f;
	EffToEnemy.bForceNewInstance = false;
	EffToEnemy.TimerHandleMethod = EStatTimerHandleMethod::STHM_UseLargerVal;
	AddStatEffsToEnemyWhenHit.Add(EffToEnemy);

	bSetGunshotSFXFromEnum = true;
	Gunshot1SFXEnum = EGunshotAudioReference::EGR_MACHINEGUN_A_1;
	Gunshot2SFXEnum = EGunshotAudioReference::EGR_MACHINEGUN_A_2;
	Gunshot3SFXEnum = EGunshotAudioReference::EGR_MACHINEGUN_A_3;
	Gunshot4SFXEnum = EGunshotAudioReference::EGR_MACHINEGUN_A_4;
	Gunshot5SFXEnum = EGunshotAudioReference::EGR_MACHINEGUN_A_5;
	Gunshot6SFXEnum = EGunshotAudioReference::EGR_MACHINEGUN_A_6;
	Gunshot7SFXEnum = EGunshotAudioReference::EGR_MACHINEGUN_A_7;
	Gunshot8SFXEnum = EGunshotAudioReference::EGR_MACHINEGUN_A_8;

	bSetOnEmptyFireSFXFromEnum = true;
	OnEmptyFireSFXEnum = EGunMiscAudioReference::EGM_AMMOEMPTY_1;
}
