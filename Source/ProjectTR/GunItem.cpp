// Copyright (C) 2024 by Haguk Kim


#include "GunItem.h"
#include "GameCharacter.h"
#include "TRHUDWidget.h"
#include "ItemData.h"
#include "GunItemData.h"
#include "GunPartComponent.h"
#include "TRDamageType.h"
#include "Kismet/GameplayStatics.h"
#include "WeaponFireComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "BulletTraceProjectile.h"
#include "Math/Color.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "DamageTypeNeutral.h"
#include "DamageTypePhysical.h"
#include "DamageTypeMagical.h"
#include "DamageTypeElemental.h"
#include "TRUtils.h"

AGunItem::AGunItem()
{
	// 기본 VFX 컨픽 바인딩
	if (!FxConfig)
	{
		static ConstructorHelpers::FObjectFinder<UFxConfig> FXFinder(TEXT(ASSET_DEFAULT_FX));
		if (FXFinder.Succeeded())
		{
			FxConfig = FXFinder.Object;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem - Unable to find default FX config asset!"));
		}
	}

	// 기본 CamShake 컨픽 바인딩
	if (!CamShakeConfig)
	{
		static ConstructorHelpers::FObjectFinder<UCamShakeConfig> CamShakeFinder(TEXT(ASSET_DEFAULT_CAMSHAKE));
		if (CamShakeFinder.Succeeded())
		{
			CamShakeConfig = CamShakeFinder.Object;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem - Unable to find default Cam shake config asset!"));
		}
	}

	// 기본 Projectile 컨픽 바인딩
	if (!ProjectileConfig)
	{
		static ConstructorHelpers::FObjectFinder<UProjectileConfig> ProjectileFinder(TEXT(ASSET_DEFAULT_PROJECTILE));
		if (ProjectileFinder.Succeeded())
		{
			ProjectileConfig = ProjectileFinder.Object;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem - Unable to find default Projectile config asset!"));
		}
	}

	// 기본 총알 컨픽 바인딩
	if (!BulletConfig)
	{
		static ConstructorHelpers::FObjectFinder<UBulletConfig> BulletFinder(TEXT(ASSET_DEFAULT_BULLET));
		if (BulletFinder.Succeeded())
		{
			BulletConfig = BulletFinder.Object;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem - Unable to find default Bullet config asset!"));
		}
	}

	// 기본 오디오 컨픽 바인딩
	if (!AudioConfig)
	{
		static ConstructorHelpers::FObjectFinder<UAudioConfig> AudioFinder(TEXT(ASSET_DEFAULT_AUDIO));
		if (AudioFinder.Succeeded())
		{
			AudioConfig = AudioFinder.Object;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem - Unable to find default Audio config asset!"));
		}
	}

	// 기본 오디오 설정 바인딩
	if (!GunSoundAttenuation)
	{
		static ConstructorHelpers::FObjectFinder<USoundAttenuation> SoundAttenFinder(TEXT(ASSET_DEFAULT_GUN_SOUND_ATTENUATION));
		if (SoundAttenFinder.Succeeded())
		{
			GunSoundAttenuation = SoundAttenFinder.Object;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem - Unable to find default gun sound attenuation asset!"));
		}
	}
	if (!GunSoundConcurrency)
	{
		static ConstructorHelpers::FObjectFinder<USoundConcurrency> SoundConFinder(TEXT(ASSET_DEFAULT_GUN_SOUND_CONCURRENCY));
		if (SoundConFinder.Succeeded())
		{
			GunSoundConcurrency = SoundConFinder.Object;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem - Unable to find default gun sound concurrency asset!"));
		}
	}

	// 투사체 및 총알 기본값 바인딩
	if (ProjectileConfig)
	{
		DefaultProjectileClass = ProjectileConfig->DefaultProjClass;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGunItem - Unable to set default Projectile since there are no projectile config asset!"))
	}

	if (BulletConfig)
	{
		DefaultBulletStruct = BulletConfig->DefaultBulletStruct;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGunItem - Unable to set default Bullet since there are no bullet config asset!"))
	}

	// 기타 기본값 지정
	if (!DamageType)
	{
		// 기본값 Neutral
		DamageType = UDamageTypeNeutral::StaticClass();
	}

	// 장탄 최대로 시작
	CurrAmmo = MaxAmmo;

#pragma region /** Component Initialization */
	// 미리 바인딩된 클래스가 없는 경우 WeaponFireComponent 클래스를 바인딩한다
	if (!PrimaryActCompClass)
	{
		PrimaryActCompClass = UWeaponFireComponent::StaticClass();
	}

	// 총기 박스 컴포넌트(루트) 초기화
	GunBoxColComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("GunBoxColComp"));
	InitGunBoxColComponent();
	// 기타 세부 설정은 파츠의 초기화가 끝나면 서버에서 처리

	// 루트 교체
	USceneComponent* OriginRoot = GetRootComponent();
	SetRootComponent(GunBoxColComponent);
	InitRootComp(GunBoxColComponent);
	OriginRoot->SetupAttachment(GunBoxColComponent);

	// 라이팅 컴포넌트 (VFX)
	// 총구 위치에 부착하는 과정은 파츠 초기화 이후 처리
	MuzzleFlashLightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlashLightComp"));
	MuzzleFlashLightComp->SetIntensity(MuzzleLightIntensity);
	MuzzleFlashLightComp->SetAttenuationRadius(1000.0f);
	MuzzleFlashLightComp->SetVisibility(false);
	MuzzleFlashLightComp->SetupAttachment(GetRootComponent());
	MuzzleFlashLightComp->SetCastShadows(false);
	MuzzleFlashLightComp->SetCastVolumetricShadow(false);

	// 나이이가라 컴포넌트 (VFX)
	// NOTE: VFX 에셋 바인딩 및 소켓 어태치의 경우 파츠 초기화 이후 일괄적으로 처리한다
	MuzzleFlashComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraMuzzleFlash"));
	MuzzleFlashComp->SetAutoActivate(false);
	TRUtils::OptimizePrimitiveComp(MuzzleFlashComp);

	ShellEjectComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraShellEject"));
	ShellEjectComp->SetAutoActivate(false);
	TRUtils::OptimizePrimitiveComp(ShellEjectComp);
#pragma endregion
}

void AGunItem::BeginPlay()
{
	Super::BeginPlay();

	// 스텟 적용
	// 서버의 경우 BeginPlay 시점에 모든 스테이터스가 최신화되었음이 보장되므로 여기서 1회만 호출해도 무방함
	// 클라의 경우에도 레플리케이션이 BeginPlay 이전에 처리된 값들에 대한 적용이 필요하므로 여기서 호출해주어야 함
	Local_ApplyGunStats();
}

void AGunItem::OnPostInitializeComponents()
{
	if (HasAuthority())
	{
		Server_InitGunParts();
	}
}

void AGunItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWeaponFireComponent* FireComp = Cast<UWeaponFireComponent>(PrimaryActComponent);
	if (FireComp)
	{
		FireComp->Local_StopFx(GetItemDeployer() /* nullptr일 수 있음 */);
	}

	Super::EndPlay(EndPlayReason);
}

void AGunItem::OnDeployerStatChanged()
{
	Super::OnDeployerStatChanged();
	Local_ApplyGunStats();

	// NOTE:
	// 구조적으로 총기 액터가 생성되기 전까지 어트리뷰트 UI는 정확한 값을 알 수 없음
	// 사실 스테이터스가 변할 때마다 액터 인스턴스를 생성해 어트리뷰트를 갱신하고 다시 파괴하면 가능은 하지만
	// 지나치게 부하가 큼
	// 고로 현실적인 해결방안은 현재 deploy중인 액터에 한정해서 '실제' 어트리뷰트를 보여주는 방식임
	// (그 외 단순 소지중인 invobject들의 경우에는 그 아이템 고유 스테이터스만 보여주는 방식)
}

void AGunItem::Server_InitGunParts()
{
	if (!HasAuthority()) return;

	// 레플리케이션 해야 할 파츠의 수 설정
	GunPartsToReplicateCount = GetGunParts().Num();

	// 메쉬 변경 및 메쉬 트리 빌드
	SetMeshToReceiver();
	ConstructMeshTree();

	// 능력치 초기화
	InitGunStats();

	// 총기 설명 추가
	GenerateWeaponAttr();

	// 공통 초기화 로직 수행
	Host_InitGunParts();
}

void AGunItem::Client_InitGunParts()
{
	if (HasAuthority()) return;

	// 메쉬 트리 빌드
	ConstructMeshTree();

	// 렌더링 결과 적용
	if (bShouldRenewItemVisibility)
	{
		SetItemVisibility(bItemVisibility);
	}

	// 공통 초기화 로직 수행
	Host_InitGunParts();
}

void AGunItem::Host_InitGunParts()
{
	// 서버와 클라이언트 모두 공통으로 처리해야 하는 로직들로,
	// 서버의 경우 OnPostInitializeComponents에서, 클라의 경우 모든 파츠의 레플리케이션이 완료된 시점에 처리한다
	
	// 파츠 메쉬 설정 초기화
	InitializeGunPartMesh();

	// 총기 크기에 맞게 콜리전 조정
	GunBoxColComponent->SetBoxExtent(GetEstimatedItemSize());

	// 총기 크기에 따라 동적으로 파지 자세 결정
	SetShouldHoldWithBothArmsBySize();

	// 파지 자세에 맞게 어태치 위치 조정
	SetRelativeAttachTransform();

	// 총구 라이트 초기 설정
	InitMuzzleLightComp();

	// 나이아가라 초기 설정
	InitVFXComp();

	// 사운드 초기 설정
	InitSFXComp();

	// 총기 크기 변화에 따른 반동 애니메이션 갱신
	RefreshRecoilAnim();
}

bool AGunItem::RestoreFromItemData(UItemData* Data)
{
	if (!Super::RestoreFromItemData(Data)) return false;
	UGunItemData* GunData = Cast<UGunItemData>(Data);
	if (!IsValid(GunData))
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to restore a gun item from a non-GunItemData %s."), *(Data->GetName()));
		return false;
	}
	if (!IsValid(GunData->GetCachedReceiverClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("%s has no receiver class cached! Aborting restore process."), *(Data->GetName()));
		return false;
	}

	// 복구 로직
	if (GunData->GetCachedReceiverClass())
	{
		SetReceiver(NewObject<UGunPartComponent>(this, GunData->GetCachedReceiverClass()));
	}
	if (GunData->GetCachedBarrelClass())
	{
		SetBarrel(NewObject<UGunPartComponent>(this, GunData->GetCachedBarrelClass()));
	}
	if (GunData->GetCachedGripClass())
	{
		SetGrip(NewObject<UGunPartComponent>(this, GunData->GetCachedGripClass()));
	}
	if (GunData->GetCachedMagazineClass())
	{
		SetMagazine(NewObject<UGunPartComponent>(this, GunData->GetCachedMagazineClass()));
	}
	if (GunData->GetCachedMuzzleClass())
	{
		SetMuzzle(NewObject<UGunPartComponent>(this, GunData->GetCachedMuzzleClass()));
	}
	if (GunData->GetCachedSightClass())
	{
		SetSight(NewObject<UGunPartComponent>(this, GunData->GetCachedSightClass()));
	}
	if (GunData->GetCachedStockClass())
	{
		SetStock(NewObject<UGunPartComponent>(this, GunData->GetCachedStockClass()));
	}

	// 기타 데이터 복구
	int32 DesiredAmmo = GunData->GetCachedCurrAmmo();
	if (DesiredAmmo < 0)
	{
		DesiredAmmo = GetStat_MaxAmmo(nullptr); // TODO: 캐릭터 레퍼런스가 전달되지 않으므로 스테이터스로 인한 효과가 제대로 반영되지 않음
	}
	Server_SetCurrAmmo(DesiredAmmo);

	return true;
}

void AGunItem::OnShotFired()
{
	Server_SetCurrAmmo(CurrAmmo - AmmoPerShot);
	if (CurrAmmo < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnShotFired - CurrAmmo went below 0. This should not happen!"));
		Server_SetCurrAmmo(0);
	}
	return;
}

void AGunItem::InitGunBoxColComponent()
{
	check(GunBoxColComponent != nullptr);

	GunBoxColComponent->SetSimulatePhysics(bShouldItemSimulatePhysics);
	GunBoxColComponent->SetEnableGravity(bShouldItemSimulateGravity);

	GunBoxColComponent->SetIsReplicated(true);
	GunBoxColComponent->bReplicatePhysicsToAutonomousProxy = true;

	GunBoxColComponent->SetCollisionProfileName(TEXT("GunBoxCol"));
	GunBoxColComponent->SetGenerateOverlapEvents(false); // 불필요
	GunBoxColComponent->SetShouldUpdatePhysicsVolume(false);
	GunBoxColComponent->SetCollisionResponseToChannel(ECC_PlayerPawn, ECollisionResponse(DefaultItemCollisionWithPawn));
	GunBoxColComponent->SetCollisionResponseToChannel(ECC_BotPawn, ECollisionResponse(DefaultItemCollisionWithPawn));
}

void AGunItem::InitMeshComp(UPrimitiveComponent* Component)
{
	check(Component != nullptr);

	// 피직스, 콜리전 모두 수행하지 않는다
	Component->SetSimulatePhysics(false);
	Component->SetEnableGravity(false);

	Component->SetCollisionProfileName(TEXT("GunPart"));
	Component->SetGenerateOverlapEvents(false); // 불필요
	Component->SetShouldUpdatePhysicsVolume(false);
}

void AGunItem::GenerateWeaponAttr()
{
	TempItemDesc = ""; // 총기는 별도 Description 없음
	
	// 주 스텟
	// 데미지
	FString DamageDesc = FString::Printf(TEXT("%d"), FMath::TruncToInt32(DmgEnemyDirect));
	if (MissileSpawnedPerShot > 1)
	{
		DamageDesc += FString::Printf(TEXT(" (x%d)"), MissileSpawnedPerShot);
	}
	TempItemAttributesForUI.Add(FItemAttribute("Damage", DamageDesc, EItemAttrType::IAT_NEUTRAL_MAJOR));

	// 데미지 타입
	// Neutral 혹은 알 수 없는 타입
	FString DamageTypeDesc = FString("Neutral");
	if (DamageType == UDamageTypePhysical::StaticClass())
	{
		DamageTypeDesc = FString("Physical");
	}
	else if (DamageType == UDamageTypeMagical::StaticClass())
	{
		DamageTypeDesc = FString("Magical");
	}
	else if (DamageType == UDamageTypeElemental::StaticClass())
	{
		DamageTypeDesc = FString("Elemental");
	}
	TempItemAttributesForUI.Add(FItemAttribute("Damage Type", DamageTypeDesc, EItemAttrType::IAT_NEUTRAL_MAJOR));

	// 발사
	FString FireModeDesc = "";
	if (FireInterval != 0.f)
	{
		FireModeDesc = FString::FromInt(FMath::TruncToInt32(100 / FireInterval));
	}
	else
	{
		FireModeDesc = "Unlimited";
	}
	TempItemAttributesForUI.Add(FItemAttribute("Fire Rate", FireModeDesc, EItemAttrType::IAT_NEUTRAL_MAJOR));

	// 장탄
	FString AmmoDesc = FString::FromInt(MaxAmmo);
	if (AmmoPerShot >= 1)
	{
		AmmoDesc += FString::Printf(TEXT(" (%d/shot)"), AmmoPerShot);
	}
	else if (AmmoPerShot < 1)
	{
		AmmoDesc += FString::Printf(TEXT(" (gain %d/shot)"), AmmoPerShot);
	}
	else
	{
		AmmoDesc = FString::Printf(TEXT(" (does not spend ammo)"));
	}
	TempItemAttributesForUI.Add(FItemAttribute("Magazine", AmmoDesc, EItemAttrType::IAT_NEUTRAL_MAJOR));

	// 반동
	FString RecoilDesc = FString::FromInt(FMath::TruncToInt32(RecoilOffsetRange * 100));
	TempItemAttributesForUI.Add(FItemAttribute("Recoil", RecoilDesc, EItemAttrType::IAT_NEUTRAL_MAJOR));

	// 부가 스텟
	// 폭발
	if (bExplodeOnHit)
	{
		TempItemAttributesForUI.Add(FItemAttribute("Explosive Rounds", "Triggers an explosion upon impact", EItemAttrType::IAT_NEUTRAL_NORMAL));
		TempItemAttributesForUI.Add(FItemAttribute("Explosion Damage", FString::FromInt(FMath::TruncToInt32(GunExplosionDamage)), EItemAttrType::IAT_NEUTRAL_MINOR));
		TempItemAttributesForUI.Add(FItemAttribute("Explosion Radius", FString::FromInt(FMath::TruncToInt32(GunExplosionRadius)), EItemAttrType::IAT_NEUTRAL_MINOR));
	}

	// 관통
	if (bHitscanPiercePawns && GunType == EWeaponFireType::WFT_HITSCAN)
	{
		FString PenBulletDesc = FString::Printf(TEXT("Bullets penetrate targets"));
		TempItemAttributesForUI.Add(FItemAttribute("Penetrating Bullets", PenBulletDesc, EItemAttrType::IAT_NEUTRAL_NORMAL));
	}
	if (bProjPiercePawns && GunType == EWeaponFireType::WFT_PROJECTILE)
	{
		FString PenProjDesc = FString::Printf(TEXT("Projectiles penetrate targets"));
		TempItemAttributesForUI.Add(FItemAttribute("Penetrating Projectiles", PenProjDesc, EItemAttrType::IAT_NEUTRAL_NORMAL));
	}

	// 헤드샷
	if (DmgMultOnHead > 1.0f)
	{
		FString HeadshotDesc = FString::Printf(TEXT("+%d%% damage on headshots"), FMath::TruncToInt32((DmgMultOnHead * 100) - 100));
		TempItemAttributesForUI.Add(FItemAttribute("Headshot Bonus", HeadshotDesc, EItemAttrType::IAT_POSITIVE));
	}

	// 에어샷
	if (DmgMultOnAirshot > 1.0f)
	{
		FString HeadshotDesc = FString::Printf(TEXT("+%d%% damage on midair targets"), FMath::TruncToInt32((DmgMultOnAirshot * 100) - 100));
		TempItemAttributesForUI.Add(FItemAttribute("Airshot Bonus", HeadshotDesc, EItemAttrType::IAT_POSITIVE));
	}

	// Falloff
	if (DmgDistFallOffMult == 1.0f)
	{
		FString FalloffDesc = FString::Printf(TEXT("No damage loss with distance"));
		TempItemAttributesForUI.Add(FItemAttribute("No Damage Falloff", FalloffDesc, EItemAttrType::IAT_POSITIVE));
	}
	else if (DmgDistFallOffMult < 1.0f)
	{
		FString FalloffDesc = FString::Printf(TEXT("Damage decreases with distance (%d%%)"), FMath::TruncToInt32((DmgDistFallOffMult * 100) - 100));
		TempItemAttributesForUI.Add(FItemAttribute("Damage Falloff", FalloffDesc, EItemAttrType::IAT_NEGATIVE));
	}
	else if (DmgDistFallOffMult > 1.0f)
	{
		FString FalloffDesc = FString::Printf(TEXT("Damage increases with distance (+%d%%)"), FMath::TruncToInt32((DmgDistFallOffMult * 100) - 100));
		TempItemAttributesForUI.Add(FItemAttribute("Long-ranged", FalloffDesc, EItemAttrType::IAT_POSITIVE));
	}

	// NOTE: 투사체 스텟은 UI에 표기하지 않는다
}

const FExplosionInfo AGunItem::GetStat_GunExplosionInfo(class AGameCharacter* Wielder) const
{
	FExplosionInfo GunExplosionInfoValue = FExplosionInfo();
	GunExplosionInfoValue.ExplosionRadius = GetStat_GunExplosionRadius(Wielder);
	GunExplosionInfoValue.BaseDamage = GetStat_GunExplosionDamage(Wielder);
	GunExplosionInfoValue.MinExplosionMultiplier = GetStat_GunMinExplosionMultiplier(Wielder);
	GunExplosionInfoValue.DmgMultOnExplInstigator = GetStat_GunDmgMultOnExplInstigator(Wielder);
	GunExplosionInfoValue.BaseImpactStrength = GetStat_GunExplImpactStrength(Wielder);
	GunExplosionInfoValue.ExplosionDamageType = GetStat_DamageType(Wielder); // TODO: Explosion damage type 따로 구분
	GunExplosionInfoValue.ExplosionVFXEnum = ExplosionVFXEnum;
	GunExplosionInfoValue.VFXRadiusConstant = ExplosionVFXRadiusConstant;

	// 고정값
	GunExplosionInfoValue.bApplyDamageOnExplosion = true;
	GunExplosionInfoValue.bApplyImpactOnExplosion = true;
	GunExplosionInfoValue.bExplodeOnBeginPlay = true;
	GunExplosionInfoValue.bDestroyAfterExplosion = true;
	GunExplosionInfoValue.ImpactFalloffType = ERadialImpulseFalloffWrapper::RIFW_Linear;
	GunExplosionInfoValue.ExplosionBlockedByType = { ECC_WorldStatic, ECC_WorldDynamic };
	GunExplosionInfoValue.ExplosionTargetType = { ECC_WorldDynamic, ECC_PlayerPawn, ECC_BotPawn, ECC_PhysicsBody, ECC_Item };

	return GunExplosionInfoValue;
}

const EWeaponFireType& AGunItem::GetStat_GunType(AGameCharacter* Wielder) const
{
	return GunType;
}

const EWeaponFireMode& AGunItem::GetStat_FireMode(AGameCharacter* Wielder) const
{
	return FireMode;
}

const TSubclassOf<class UTRDamageType>& AGunItem::GetStat_DamageType(AGameCharacter* Wielder) const
{
	return DamageType;
}

const int32 AGunItem::GetStat_MissileSpawnedPerShot(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaMissileSpawnedPerShot + MissileSpawnedPerShot;
	}
	return MissileSpawnedPerShot;
}

const bool AGunItem::GetStat_SetAccurateOffsetForFirstMissile(AGameCharacter* Wielder) const
{
	return bSetAccurateOffsetForFirstMissile;
}

const float AGunItem::GetStat_DmgEnemyDirect(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaDmgEnemyDirect + DmgEnemyDirect;
	}
	return DmgEnemyDirect;
}

const float AGunItem::GetStat_DmgAllyDirect(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaDmgAllyDirect + DmgAllyDirect;
	}
	return DmgAllyDirect;
}

const float AGunItem::GetStat_SelfDmgOnCharacterHit(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaSelfDmgOnCharacterHit + SelfDmgOnCharacterHit;
	}
	return SelfDmgOnCharacterHit;
}

const float AGunItem::GetStat_SelfDmgOnCharacterMissedHitscan(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaSelfDmgOnCharacterMissedHitscan + SelfDmgOnCharacterMissedHitscan;
	}
	return SelfDmgOnCharacterMissedHitscan;
}

const float AGunItem::GetStat_SelfDmgOnCharacterKilled(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaSelfDmgOnCharacterKilled + SelfDmgOnCharacterKilled;
	}
	return SelfDmgOnCharacterKilled;
}

const float AGunItem::GetStat_DmgMultDistClose(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaDmgMultDistClose + DmgMultDistClose;
	}
	return DmgMultDistClose;
}

const float AGunItem::GetStat_DmgMultDistFar(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaDmgMultDistFar + DmgMultDistFar;
	}
	return DmgMultDistFar;
}

const float AGunItem::GetStat_DmgMultOnAirshot(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaDmgMultOnAirshot + DmgMultOnAirshot;
	}
	return DmgMultOnAirshot;
}

const bool AGunItem::GetStat_HitscanPiercePawns(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().bSetHitscanPiercePawnsToTrue || bHitscanPiercePawns;
	}
	return bHitscanPiercePawns;
}

const float AGunItem::GetStat_GunExplosionRadius(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaGunExplosionRadius + GunExplosionRadius;
	}
	return GunExplosionRadius;
}

const float AGunItem::GetStat_GunExplosionDamage(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaGunExplosionDamage + GunExplosionDamage;
	}
	return GunExplosionDamage;
}

const float AGunItem::GetStat_GunMinExplosionMultiplier(AGameCharacter* Wielder) const
{
	return GunMinExplosionMultiplier;
}

const float AGunItem::GetStat_GunDmgMultOnExplInstigator(AGameCharacter* Wielder) const
{
	return GunDmgMultOnExplInstigator;
}

const float AGunItem::GetStat_GunExplImpactStrength(AGameCharacter* Wielder) const
{
	return GunExplImpactStrength;
}

const bool AGunItem::GetStat_ExplodeOnHit(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().bSetExplodeOnHitToTrue || bExplodeOnHit;
	}
	return bExplodeOnHit;
}

const float AGunItem::GetStat_DmgMultOnHead(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaDmgMultOnHead + DmgMultOnHead;
	}
	return DmgMultOnHead;
}

const float AGunItem::GetStat_DmgDistFallOffMult(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaDmgDistFallOffMult + DmgDistFallOffMult;
	}
	return DmgDistFallOffMult;
}

const bool AGunItem::GetStat_HasDmgDistFallOff(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		if (Wielder->GetStatusDeltaSum().bSetHasDmgDistFallOffToFalse || !bHasDmgDistFallOff) return false;
		else return true;
	}
	return bHasDmgDistFallOff;
}

const bool AGunItem::GetStat_ApplyStatEffToEnemyOnHit(AGameCharacter* Wielder) const
{
	return bApplyStatEffToEnemyOnHit;
}

const bool AGunItem::GetStat_ApplyStatEffToAllyOnHit(AGameCharacter* Wielder) const
{
	return bApplyStatEffToAllyOnHit;
}

const TArray<FStatEffectGenInfo>& AGunItem::GetStat_StatEffsToEnemyWhenHit(AGameCharacter* Wielder) const
{
	return StatEffsToEnemyWhenHit;
}

void AGunItem::AddStat_StatEffToEnemyWhenHit(const TArray<FStatEffectGenInfo>& Value)
{
	for (const FStatEffectGenInfo& Effect : Value)
	{
		StatEffsToEnemyWhenHit.Add(Effect);
	}
	Local_ApplyGunStats();
}

const TArray<FStatEffectGenInfo>& AGunItem::GetStat_StatEffsToAllyWhenHit(AGameCharacter* Wielder) const
{
	return StatEffsToAllyWhenHit;
}

void AGunItem::AddStat_StatEffToAllyWhenHit(const TArray<FStatEffectGenInfo>& Value)
{
	for (const FStatEffectGenInfo& Effect : Value)
	{
		StatEffsToAllyWhenHit.Add(Effect);
	}
	Local_ApplyGunStats();
}

const float AGunItem::GetStat_FireInterval(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaFireInterval + FireInterval;
	}
	return FireInterval;
}

const float AGunItem::GetStat_RecoilOffsetRange(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaRecoilOffsetRange + RecoilOffsetRange;
	}
	return RecoilOffsetRange;
}

const int32 AGunItem::GetStat_MaxAmmo(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaMaxAmmo + MaxAmmo;
	}
	return MaxAmmo;
}

const int32 AGunItem::GetStat_AmmoPerShot(AGameCharacter* Wielder) const
{
	return AmmoPerShot;
}

const bool AGunItem::GetStat_ApplyImpactOnHit(AGameCharacter* Wielder) const
{
	return bApplyImpactOnHit;
}

const float AGunItem::GetStat_MissileMass(AGameCharacter* Wielder) const
{
	return MissileMass;
}

const TSubclassOf<class ABaseProjectile>& AGunItem::GetStat_ProjectileClass(AGameCharacter* Wielder) const
{
	return ProjectileClass;
}

const TSubclassOf<class ABaseProjectile>& AGunItem::GetStat_DefaultProjectileClass(AGameCharacter* Wielder) const
{
	return DefaultProjectileClass;
}

const float AGunItem::GetStat_ProjInitialSpeed(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaProjSpeed + ProjInitialSpeed;
	}
	return ProjInitialSpeed;
}

const float AGunItem::GetStat_ProjMaxSpeed(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().DeltaProjSpeed + ProjMaxSpeed;
	}
	return ProjMaxSpeed;
}

const bool AGunItem::GetStat_ProjRotationFollowsVelocity(AGameCharacter* Wielder) const
{
	return bProjRotationFollowsVelocity;
}

const bool AGunItem::GetStat_ProjRotationRemainsVertical(AGameCharacter* Wielder) const
{
	return bProjRotationRemainsVertical;
}

const bool AGunItem::GetStat_ProjShouldBounce(AGameCharacter* Wielder) const
{
	return bProjShouldBounce;
}

const float AGunItem::GetStat_ProjBounciness(AGameCharacter* Wielder) const
{
	return ProjBounciness;
}

const bool AGunItem::GetStat_ProjBounceOffObjIndefinitely(AGameCharacter* Wielder) const
{
	return bProjBounceOffObjIndefinitely;
}

const int32 AGunItem::GetStat_ProjDestroyOnHitCount(AGameCharacter* Wielder) const
{
	return ProjDestroyOnHitCount;
}

const bool AGunItem::GetStat_ProjPiercePawns(AGameCharacter* Wielder) const
{
	if (Wielder)
	{
		return Wielder->GetStatusDeltaSum().bSetProjPiercePawnsToTrue || bProjPiercePawns;
	}
	return bProjPiercePawns;
}

const float AGunItem::GetStat_ProjGravityScale(AGameCharacter* Wielder) const
{
	return ProjGravityScale;
}

void AGunItem::OnRep_AmmoPerShot()
{
	Client_PredictedAmmoPerShot = AmmoPerShot;
}

bool AGunItem::Host_CanFireShot()
{
	if (!PrimaryActComponent)
	{
		return false;
	}

	if (CurrAmmo - AmmoPerShot >= 0)
	{
		return true;
	}
	return false;
}

void AGunItem::Server_SetCurrAmmo(int32 Value)
{
	if (!HasAuthority()) return;
	CurrAmmo = Value;

	// 서버의 경우 수동 호출
	Local_OnCurrAmmoUpdated();
}

void AGunItem::OnRep_CurrAmmo()
{
	Client_PredictedCurrAmmo = CurrAmmo;

	Local_OnCurrAmmoUpdated();
}

void AGunItem::Local_OnCurrAmmoUpdated()
{
	// UI 업데이트 (서버,클라)
	AGameCharacter* GameChar = GetItemDeployer();
	if (GameChar && GameChar->Local_GetBoundHUDWidget().IsValid())
	{
		GameChar->Local_GetBoundHUDWidget()->UpdateAmmo();
	}
}

void AGunItem::InitGunStats()
{
	// 리시버 배럴 그립 매거진 사이트 스톡 머즐 순서
	TArray<UGunPartComponent*> Parts = GetGunParts();

	// 패스 1
	for (UGunPartComponent* Part : Parts)
	{
		if (!IsValid(Part)) continue;
		else
		{
			Part->StatPass1(this);
		}
	}

	// 패스 2
	for (UGunPartComponent* Part : Parts)
	{
		if (!IsValid(Part)) continue;
		else
		{
			Part->StatPass2(this);
		}
	}

	// 패스 3
	for (UGunPartComponent* Part : Parts)
	{
		if (!IsValid(Part)) continue;
		else
		{
			Part->StatPass3(this);
		}
	}
	
	// 검증 단계
	// 특정 범위 내에 속해야 하는 값들을 처리한다
	ValidateStats();
}

void AGunItem::ValidateStats()
{
	// 최소한 1 이상의 최대장탄을 가져야 함
	SetStat_MaxAmmo(FMath::Max(MaxAmmo, GunConst::GUN_MIN_MAXAMMO));

	// 탄약을 음수만큼 소비할 수 없음
	SetStat_AmmoPerShot(FMath::Max(AmmoPerShot, GunConst::GUN_MIN_AMMOPERSHOT));

	// 최소 하나 이상의 대상을 발사
	SetStat_MissileSpawnedPerShot(FMath::Max(MissileSpawnedPerShot, GunConst::GUN_MIN_MISSILEPERSHOT));

	// 게이지 범위 지정
	GunGauge = FMath::Clamp(GunGauge, GunConst::GUN_MIN_GAUGE, GunConst::GUN_MAX_GAUGE);

	// 폭발의 피해 배율은 음수가 될 수 없다
	// 만약 폭발이 주위 팀원을 치료하는 것을 의도했다면 이는 피해 배율이 아닌 ExplosionDmg 값을 통해 처리되어야 한다
	SetStat_GunDmgMultOnExplInstigator(FMath::Max(GunDmgMultOnExplInstigator, 0.0f));
	SetStat_GunMinExplosionMultiplier(FMath::Max(GunMinExplosionMultiplier, 0.0f));

	// 기본값이 존재하는 값들의 경우 건파츠들에 의해 바인딩되지 않았을 경우 기본값을 사용한다
	if (!bBulletStructValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateStats - Bullet struct is not bound. Using default value."));
		BulletStruct = DefaultBulletStruct;
	}
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateStats - Projectile class is not bound. Using default value."));
		ProjectileClass = DefaultProjectileClass;
	}

	if (bProjPiercePawns && bProjShouldBounce)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValidateStats - Projectile cannot pierce pawns while being able to bounce. Check TRProjMovementComponent.h for details."));
	}

	// TODO: 필요 시 추가
}

void AGunItem::Local_ApplyGunStats()
{
	// NOTE:
	// 여기서 바인딩되는 값들의 Setter에서는 반드시 이 함수를 호출해주어야 한다

	UWeaponFireComponent* FireComp = Cast<UWeaponFireComponent>(PrimaryActComponent);
	if (FireComp)
	{
		/* 클라이언트 FX에 영향을 주기 때문에 레플리케이션 되는 값들 */
		AGameCharacter* Deployer = GetItemDeployer();

		FireComp->CurrGunType = GetStat_GunType(Deployer);
		FireComp->CurrFireMode = GetStat_FireMode(Deployer);
		FireComp->CurrDamageType = GetStat_DamageType(Deployer);
		FireComp->CurrRapidFireInterval = GetStat_FireInterval(Deployer);
		FireComp->TriggerInterval = GetStat_FireInterval(Deployer);
		FireComp->CurrRecoilOffsetRange = GetStat_RecoilOffsetRange(Deployer);
		FireComp->FireMissilePerShot = GetStat_MissileSpawnedPerShot(Deployer);
		FireComp->bFireSetAccurateOffsetForFirstMissile = GetStat_SetAccurateOffsetForFirstMissile(Deployer);
		FireComp->FireProjClass = GetStat_ProjectileClass(Deployer);

		FireComp->bApplyStatEffToEnemyOnHit = GetStat_ApplyStatEffToEnemyOnHit(Deployer);
		FireComp->StatEffsToEnemyWhenHit = GetStat_StatEffsToEnemyWhenHit(Deployer);
		FireComp->bApplyStatEffToAllyOnHit = GetStat_ApplyStatEffToAllyOnHit(Deployer);
		FireComp->StatEffsToAllyWhenHit = GetStat_StatEffsToAllyWhenHit(Deployer);

		// VFX
		FireComp->FireBulletStruct = BulletStruct;
		FireComp->FireCameraShakeClass = CameraShakeClass;

		/* CurrRapidFireInterval 등 일부 값의 변경에 의한 반동 애니메이션 데이터 갱신 */
		// NOTE: 클라의 경우 아직 값이 도착하지 않았을 가능성이 있다; 
		// 클라는 ReplicateUsing을 사용해 해당 값(들)이 변경될 때 다시 호출한다
		RefreshRecoilAnim();
	}

	if (MuzzleFlashLightComp)
	{
		// 라이트 색상 및 세기
		MuzzleFlashLightComp->SetLightFColor(MuzzleLightColor);
		MuzzleFlashLightComp->SetIntensity(MuzzleLightIntensity);
	}
}

TArray<UGunPartComponent*> AGunItem::GetGunParts()
{
	TArray<UGunPartComponent*> GunPartsArray;
	if (ReceiverComp) GunPartsArray.Add(ReceiverComp);
	if (BarrelComp) GunPartsArray.Add(BarrelComp);
	if (GripComp) GunPartsArray.Add(GripComp);
	if (MagazineComp) GunPartsArray.Add(MagazineComp);
	if (SightComp) GunPartsArray.Add(SightComp);
	if (StockComp) GunPartsArray.Add(StockComp);
	if (MuzzleComp) GunPartsArray.Add(MuzzleComp);
	return GunPartsArray;
}

void AGunItem::SetMeshToReceiver()
{
	if (IsValid(GetReceiver()) && IsValid(GetReceiver()->GetMeshComp()))
	{
		TArray<USceneComponent*> ChildrenToTransfer;
		if (MeshComponent)
		{
			// 기존 Mesh Component에 부착되어있는 모든 자식 컴포넌트를 탈착
			for (USceneComponent* ChildComponent : MeshComponent->GetAttachChildren())
			{
				ChildrenToTransfer.Add(ChildComponent);
			}
			for (USceneComponent* ChildComponent : ChildrenToTransfer)
			{
				ChildComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			}

			// 기존 Mesh Component 제거
			if (IsValid(MeshComponent->GetAttachParent()))
			{
				MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			}
			MeshComponent = nullptr;
		}

		// 메쉬를 리시버 메쉬로 교환한다
		MeshComponent = GetReceiver()->GetMeshComp();
		MeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		// 기존 자식 컴포넌트들을 새 Mesh Component에 부착
		for (USceneComponent* ChildComponent : ChildrenToTransfer)
		{
			ChildComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			ChildComponent->SetWorldLocation(MeshComponent->GetComponentLocation());
			ChildComponent->SetWorldRotation(MeshComponent->GetComponentRotation());
		}
		return;
	}

	// 디버그 로그
	bool ReceiverValidity = IsValid(GetReceiver());
	if (!GetReceiver())
	{
		UE_LOG(LogTemp, Error, TEXT("AGunItem::SetMeshToReceiver() - Cannot access to mesh component of %s. Receiver is null! / Auth : %d"), *GetName(), HasAuthority());
		return;
	}
	bool ReceiverMeshValidity = IsValid(GetReceiver()->GetMeshComp());
	if (!GetReceiver()->GetMeshComp())
	{
		UE_LOG(LogTemp, Error, TEXT("AGunItem::SetMeshToReceiver() - Cannot access to mesh component of %s. Receiver's Mesh component is null! / Auth : %d"), *GetName(), HasAuthority());
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("AGunItem::SetMeshToReceiver() - Cannot access to mesh component of %s. ReceiverComp Validity: %d, Receiver->MeshComp Validity: %d / Auth : %d"), *GetName(), ReceiverValidity, ReceiverMeshValidity, HasAuthority());
	if (!ReceiverValidity && GetReceiver())
	{
		UE_LOG(LogTemp, Error, TEXT("Receiver invalid but not null: Garbage: %d PendingKill: %d"), GetReceiver()->HasAnyFlags(RF_InternalGarbage), GetReceiver()->HasAnyFlags(RF_InternalPendingKill));
	}
	if (!ReceiverMeshValidity && GetReceiver()->GetMeshComp())
	{
		UE_LOG(LogTemp, Error, TEXT("Receiver invalid but not null: Garbage: %d PendingKill: %d"), GetReceiver()->GetMeshComp()->HasAnyFlags(RF_InternalGarbage), GetReceiver()->GetMeshComp()->HasAnyFlags(RF_InternalPendingKill));
	}
	return;
}

bool AGunItem::CheckComponentMeshHasSockets(UGunPartComponent* Comp, TArray<FName> SocketNames)
{
	if (!Comp || !(Comp->GetMeshComp()))
	{
		UE_LOG(LogTemp, Error, TEXT("CheckComponentMeshHasSocket - Invalid argument. / Auth : %d"), HasAuthority());
		return false;
	}

	bool result = true;
	for (const FName& Name : SocketNames)
	{
		if (!Comp->GetMeshComp()->DoesSocketExist(Name))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s 's mesh component does not have socket %s!"), *(Comp->GetName()), *(Name.ToString()));
			result = false;
		}
	}
	return result;
}

void AGunItem::InitGunPartMeshComp(UPrimitiveComponent* Mesh)
{
	if (!Mesh) return;
	InitMeshComp(Mesh);
}

void AGunItem::InitializeGunPartMesh()
{
	TArray<UGunPartComponent*> Parts = GetGunParts();
	for (UGunPartComponent* Part : Parts)
	{
		InitGunPartMeshComp(Part->GetMeshComp());
	}
}

void AGunItem::ResizeReachCompToMatchItem()
{
	Super::ResizeReachCompToMatchItem();
	// TODO: 각 파츠들의 메쉬 크기도 고려해 조정
}

void AGunItem::OnRep_GunPartComp()
{
	if (!HasAuthority())
	{
		// 모든 파츠를 다 수신했다면 모든 동적 오브젝트가 최신화된 상태이므로 후처리를 수행한다
		if (GetGunParts().Num() >= GunPartsToReplicateCount)
		{
			bClient_AllPartsReplicated = true;

			Client_InitGunParts();
		}
	}
}

void AGunItem::SetRelativeAttachTransform()
{
	if (ShouldHoldWithBothArms())
	{
		if (GripComp)
		{
			this->AttachRelativeLocation = GripComp->GetRelativeLocation();
		}
		this->AttachRelativeLocation += FVector(-20.0f, 0.0f, 5.0f);
		this->AttachRelativeRotation = FRotator(0.0f, 90.0f, 10.0f);
	}
	else
	{
		if (GripComp)
		{
			this->AttachRelativeLocation = GripComp->GetRelativeLocation();
		}
		this->AttachRelativeLocation += FVector(-20.0f, 7.0f, 8.0f);
		this->AttachRelativeRotation = FRotator(0.0f, 70.0f, 8.0f);
	}
}

FVector AGunItem::GetEstimatedItemSize()
{
	FVector ReceiverSize = FVector(0, 0, 0);
	FVector BarrelSize = FVector(0, 0, 0);
	FVector StockSize = FVector(0, 0, 0);
	FVector SightSize = FVector(0, 0, 0);
	FVector GunSize = FVector(0, 0, 0);

	if (GetReceiver() && GetReceiver()->GetMeshComp()) 
		ReceiverSize = GetReceiver()->GetMeshComp()->GetLocalBounds().BoxExtent;
	if (GetBarrel() && GetBarrel()->GetMeshComp())
		BarrelSize = GetBarrel()->GetMeshComp()->GetLocalBounds().BoxExtent;
	if (GetStock() && GetStock()->GetMeshComp())
		StockSize = GetStock()->GetMeshComp()->GetLocalBounds().BoxExtent;
	if (GetSight() && GetSight()->GetMeshComp())
		SightSize = GetSight()->GetMeshComp()->GetLocalBounds().BoxExtent;

	GunSize.Y = (ReceiverSize.Y + BarrelSize.Y + StockSize.Y); // 파츠간 겹치는 부분이 있으므로 오차 발생 가능
	GunSize.X = FMath::Max3(ReceiverSize.X, BarrelSize.X, StockSize.X);
	GunSize.Z = FMath::Max3(ReceiverSize.Z, BarrelSize.Z, StockSize.Z) + SightSize.Z;
	//UE_LOG(LogTemp, Error, TEXT("GSIZE %f %f %f"), GunSize.X, GunSize.Y, GunSize.Z);
	return GunSize;
}

void AGunItem::ConstructMeshTree()
{
	if (!IsValid(GetReceiver()))
	{
		UE_LOG(LogTemp, Error, TEXT("AGunItem::ConstructMeshTree() - %s's receiver is invalid! / Auth : %d"), *GetName(), HasAuthority());
		return;
	}
	if (!IsValid(GetReceiver()->GetMeshComp()))
	{
		UE_LOG(LogTemp, Error, TEXT("AGunItem::ConstructMeshTree() - %s's receiver mesh component is invalid! / Auth : %d"), *GetName(), HasAuthority());
		return;
	}

	// 파츠 검증
	CheckComponentMeshHasSockets(GetReceiver(), { BARREL_SOCKET, SIGHT_SOCKET, STOCK_SOCKET, GRIP_SOCKET, MAG_SOCKET });
	CheckComponentMeshHasSockets(GetBarrel(), { MUZZLE_SOCKET, SIGHT_SOCKET });
	// 소켓이 없더라도 프로세스 자체를 중단하지는 않음

	// 파츠 부착
	// 배럴
	AttachToPart(GetReceiver(), GetBarrel(), BARREL_SOCKET);
	// 그립
	AttachToPart(GetReceiver(), GetGrip(), GRIP_SOCKET);
	// 매거진
	AttachToPart(GetReceiver(), GetMagazine(), MAG_SOCKET);
	// 머즐
	AttachToPart(GetBarrel(), GetMuzzle(), MUZZLE_SOCKET);
	// 사이트
	AttachToPart(GetReceiver(), GetSight(), SIGHT_SOCKET); // TODO: 배럴 사이트로 부착할 수도 있음
	// 스톡
	AttachToPart(GetReceiver(), GetStock(), STOCK_SOCKET); // NOTE: 스켈레톤, 스태틱 둘 다 허용
	// 어태치먼트
	// TODO

	return;
}

bool AGunItem::AttachToPart(UGunPartComponent* Parent, UGunPartComponent* Comp, FName SocketName)
{
	if (!IsValid(Parent))
	{
		UE_LOG(LogTemp, Error, TEXT("AGunItem::AttachToPart() - Item %s - Parent comp is not valid. / Auth : %d"), *GetName(), HasAuthority());
		return false;
	}
	if (!IsValid(Comp))
	{
		// 자식이 invalid한 경우는 정상적인 흐름에서도 발생할 수 있으므로 별도의 로그를 출력하지 않음
		return false;
	}
	if (!IsValid(Comp->GetMeshComp()))
	{
		UE_LOG(LogTemp, Error, TEXT("AGunItem::AttachToPart() - Item %s - Child comp %s's mesh is not valid. / Auth : %d"), *GetName(), *(Comp->GetName()), HasAuthority());
		return false;
	}

	USkeletalMeshComponent* ParentSK = Cast<USkeletalMeshComponent>(Parent->GetMeshComp());
	UStaticMeshComponent* ParentSM = Cast<UStaticMeshComponent>(Parent->GetMeshComp());
	if (IsValid(ParentSK))
	{
		if (!Comp->GetMeshComp()->AttachToComponent(ParentSK, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName))
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem::AttachToPart() - Item %s - Failed to attach %s to %s properly. This can happen if you are attaching a physics object to another physics object. To avoid this, you can toggle off one of the object's bSimaltePhysics. / Auth: %d"), *GetName(), *(Parent->GetName()), *(Comp->GetName()), HasAuthority());
			return false;
		}
		Comp->AddLocalOffset(Comp->GetMeshOffset());
	}
	else if (IsValid(ParentSM))
	{
		if (!Comp->GetMeshComp()->AttachToComponent(ParentSM, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName))
		{
			UE_LOG(LogTemp, Error, TEXT("AGunItem::AttachToPart() - Item %s - Failed to attach %s to %s properly. This can happen if you are attaching a physics object to another physics object. To avoid this, you can toggle off one of the object's bSimaltePhysics. / Auth: %d"), *GetName(), *(Parent->GetName()), *(Comp->GetName()), HasAuthority());
			return false;
		}
		Comp->AddLocalOffset(Comp->GetMeshOffset());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGunItem::ConstructMeshTree() - Item %s - Parent comp %s's mesh is not valid. / Auth : %d"), *GetName(), *(Parent->GetName()), HasAuthority());
		return false;
	}

	return true;
}

void AGunItem::SetShouldHoldWithBothArmsBySize()
{
	if (GetEstimatedItemSize().Y > YLenLimitToHoldWithBothArms)
	{
		bShouldHoldWithBothArms = true;
	}
}

void AGunItem::RegisterVisibility(bool bVisibility)
{
	// 서버이거나 혹은 클라이언트인데 이미 모든 파츠를 수신받았을 경우 즉각 처리한다
	if (HasAuthority() || bClient_AllPartsReplicated)
	{
		SetItemVisibility(bVisibility);
		return;
	}

	// 지금 바로 처리를 하지 못하는 경우 대기
	bShouldRenewItemVisibility = true;
	bItemVisibility = bVisibility;
}

void AGunItem::SetItemVisibility(bool bVisibility)
{
	Super::SetItemVisibility(bVisibility);
	TArray<UGunPartComponent*> GunParts = GetGunParts();

	for (UGunPartComponent* GunPart : GunParts)
	{
		if (GunPart)
		{
			if (GunPart->GetMeshComp())
			{
				GunPart->GetMeshComp()->SetVisibility(bVisibility, true);
			}
		}
	}
}

void AGunItem::SpawnMuzzleFlashVFX()
{
	if (MuzzleFlashComp)
	{
		MuzzleFlashComp->ActivateSystem(false);
	}
}

void AGunItem::FlashMuzzleLight(float Duration)
{
	if (MuzzleFlashLightComp && !MuzzleFlashLightComp->IsVisible())
	{
		MuzzleFlashLightComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(MuzzleFlashTimerHandle, this, &AGunItem::DisableMuzzleLight, Duration, false);
	}
}

void AGunItem::DisableMuzzleLight()
{
	if (MuzzleFlashLightComp)
	{
		MuzzleFlashLightComp->SetVisibility(false);
	}
}

void AGunItem::SpawnShellEjectVFX()
{
	if (ShellEjectComp)
	{
		ShellEjectComp->ActivateSystem(false);
	}
}

TPair<USceneComponent*, FName> AGunItem::GetMuzzleCompAndSock()
{
	UGunPartComponent* TargetComp = GetBarrel();
	FName TargetSock = MUZZLE_SOCKET;
	if (!TargetComp)
	{
		TargetComp = GetReceiver();
		TargetSock = BARREL_SOCKET;
		if (!TargetComp)
		{
			UE_LOG(LogTemp, Error, TEXT("GetMuzzleCompAndSock - Could not find any valid gun parts!"));
			return { nullptr, FName("Invalid") };
		}
	}
	return { TargetComp->GetMeshComp(), TargetSock };
}

void AGunItem::InitMuzzleLightComp()
{
	// 부착 위치 조정
	TPair<USceneComponent*, FName> MuzzleInfo = GetMuzzleCompAndSock();
	if (MuzzleInfo.Get<0>())
	{
		MuzzleFlashLightComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		MuzzleFlashLightComp->AttachToComponent(MuzzleInfo.Get<0>(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, MuzzleInfo.Get<1>());
	}
	else
	{
		// 총구를 찾지 못한 경우 루트 위치를 사용한다
		UE_LOG(LogTemp, Warning, TEXT("InitMuzzleLightComp - LightComponent failed to find appropriate muzzle info!"));
	}

	// 총기 자체에 빛이 비치게끔 오프셋 조정
	MuzzleFlashLightComp->SetRelativeLocation(FVector(0.0f, 40.0f, 20.0f));
}

void AGunItem::InitVFXComp()
{
	if (MuzzleFlashComp)
	{
		USceneComponent* AttachTarget = GetBarrel() ? GetBarrel()->GetMeshComp() : nullptr;
		if (AttachTarget)
		{
			MuzzleFlashComp->AttachToComponent(AttachTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MUZZLE_SOCKET);
			MuzzleFlashComp->SetRelativeRotation(FVector::RightVector.Rotation());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Local_ApplyGunStats - Invalid barrel; Failed to attach muzzle vfx niagara component"));
			MuzzleFlashComp->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}

	if (ShellEjectComp)
	{
		USceneComponent* AttachTarget = GetReceiver() ? GetReceiver()->GetMeshComp() : nullptr;
		if (AttachTarget)
		{
			ShellEjectComp->AttachToComponent(AttachTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			ShellEjectComp->SetRelativeLocation(FVector::RightVector * -1.0f); // 총기 좌측에서 우측 전방 상단을 향해 생성; 1인칭 모델에서 보이게 하기 위함
			ShellEjectComp->SetRelativeRotation((FVector::RightVector + FVector::ForwardVector + FVector::DownVector).Rotation());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Local_ApplyGunStats - Invalid receiver; Failed to attach shell vfx niagara component"));
			ShellEjectComp->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}

	// 서버의 경우 여기서 직접 호출하는 것으로 처리
	// 클라의 경우 파츠들의 레플리케이션이 먼저 오고 VFX가 나중에 올 경우 ReplicatedUsing으로 처리
	// 그렇지 않고 VFX가 먼저 올경우 여기서 처리
	OnRep_VFXAssetChanged();
}

void AGunItem::OnRep_VFXAssetChanged()
{
	if (ShellEjectVFX) ShellEjectComp->SetAsset(ShellEjectVFX);
	if (MuzzleFlashVFX) MuzzleFlashComp->SetAsset(MuzzleFlashVFX);
}

void AGunItem::OnRep_SFXAssetChanged()
{
	Local_GunshotSFXCandidates.Empty();
	if (Gunshot1SFX) Local_GunshotSFXCandidates.Add(Gunshot1SFX);
	if (Gunshot2SFX) Local_GunshotSFXCandidates.Add(Gunshot2SFX);
	if (Gunshot3SFX) Local_GunshotSFXCandidates.Add(Gunshot3SFX);
	if (Gunshot4SFX) Local_GunshotSFXCandidates.Add(Gunshot4SFX);
	if (Gunshot5SFX) Local_GunshotSFXCandidates.Add(Gunshot5SFX);
	if (Gunshot6SFX) Local_GunshotSFXCandidates.Add(Gunshot6SFX);
	if (Gunshot7SFX) Local_GunshotSFXCandidates.Add(Gunshot7SFX);
	if (Gunshot8SFX) Local_GunshotSFXCandidates.Add(Gunshot8SFX);
}

USoundCue* AGunItem::Local_GetRandomGunshotSFX()
{
	if (!Local_GunshotSFXCandidates.IsEmpty())
	{
		return Local_GunshotSFXCandidates[FMath::Rand() % Local_GunshotSFXCandidates.Num()];
	}
	UE_LOG(LogTemp, Error, TEXT("Local_GetRandomGunshotSFX - Failed to return a valid sfx!"));
	return nullptr;
}

USoundCue* AGunItem::Local_GetRandomEmptyFireSFX()
{
	return OnEmptyFireSFX;
}

void AGunItem::Local_PlayGunshotSFX(bool bIsAmmoEmpty, float Volume, float Pitch)
{
	USoundCue* SFX = bIsAmmoEmpty ? Local_GetRandomEmptyFireSFX() : Local_GetRandomGunshotSFX();
	if (!SFX)
	{
		UE_LOG(LogTemp, Error, TEXT("Local_PlayGunshotSFX - Invalid gunshot sfx!"));
		return;
	}

	USceneComponent* AttachTarget = nullptr;
	if (GetBarrel())
	{
		AttachTarget = GetBarrel()->GetMeshComp();
	}
	if (!AttachTarget)
	{
		AttachTarget = this->GetRootComponent();
	}

	UGameplayStatics::SpawnSoundAttached(
		SFX, 
		AttachTarget, 
		MUZZLE_SOCKET,
		FVector::ZeroVector,
		EAttachLocation::SnapToTarget,
		false, // 총기 액터가 파괴되어도 소리가 도중에 멈추지는 않는다
		Volume,
		Pitch, 
		0.0f, 
		GunSoundAttenuation, 
		GunSoundConcurrency,
		true // 재생 완료 시 자동 파괴
	);
}

void AGunItem::InitSFXComp()
{
	if (!GunSoundAttenuation)
	{
		UE_LOG(LogTemp, Error, TEXT("InitSFXComp - GunSoundAttenuation is missing!"));
	}
	if (!GunSoundConcurrency)
	{
		UE_LOG(LogTemp, Error, TEXT("InitSFXComp - GunSoundConcurrency is missing!"));
	}

	// 서버의 경우 여기서 직접 호출
	OnRep_SFXAssetChanged();
}

void AGunItem::RefreshRecoilAnim()
{
	// 총기 크기에 따라 반동값 변경
	if (ShouldHoldWithBothArms())
	{
		RecoilAnimData.StoredData = HeavyRecoilAsset;
		RecoilAnimData.SingleLoc = HeavyLocVector;
		RecoilAnimData.SingleRot = HeavyRotVector;
	}
	else
	{
		RecoilAnimData.StoredData = LightRecoilAsset;
		RecoilAnimData.SingleLoc = LightLocVector;
		RecoilAnimData.SingleRot = LightRotVector;
	}

	// 반동 주기, 버스트 값 재계산
	UWeaponFireComponent* FireComp = Cast<UWeaponFireComponent>(PrimaryActComponent);
	if (FireComp)
	{
		float RecoilInterval = FMath::Max(FireComp->CurrRapidFireInterval, 0.001f);

		// NOTE: PRAS 자체적으로 설정된 최소값은 0.001이지만, 낮은 프레임에서의 튕김 현상을 막기 위해 조금 더 보수적으로 설정함
		// 프레임 당 소요시간이 해당 값을 넘어설때 튕김 현상이 발생하는 것으로 추정됨 
		// RecoilComp::Init() 참고
		RecoilRate = FMath::Max(60 / RecoilInterval, 0.05f);
	}

	// 현재 Deploy된 상태라면 정보를 반영한다
	AGameCharacter* ItemOwner = GetItemDeployer();
	if (ItemOwner)
	{
		ItemOwner->Local_InitRecoil(RecoilAnimData, RecoilRate, RecoilBurst);
	}
}
