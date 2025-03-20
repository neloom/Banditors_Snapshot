// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "WieldItem.h"
#include "TRStructs.h"
#include "TRMacros.h"
#include "TRExplosion.h"
#include "CamShakeConfig.h"
#include "FxConfig.h"
#include "AudioConfig.h"
#include "ProjectileConfig.h"
#include "BulletConfig.h"
#include "StatusEffect.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "Net/UnrealNetwork.h"
#include "GunItem.generated.h"

struct GunConst
{
	static constexpr int GUN_MIN_MAXAMMO = 1;
	static constexpr int GUN_MIN_AMMOPERSHOT = 0;
	static constexpr int GUN_MIN_MISSILEPERSHOT = 1;
	static constexpr float GUN_MIN_GAUGE = 0.0f;
	static constexpr float GUN_MAX_GAUGE = 1.0f;
	static constexpr float GUN_MAX_FALLOFF_DIST = 2500.0f;
	static constexpr float GUN_MIN_FALLOFF_DIST = 250.0f;
};

/**
 * GunItem은 레벨에 미리 배치해둔 상태로 사용할 수 있는 액터가 아니며,
 * 반드시 GunGeneration 프로세스를 거쳐야만 정상적인 사용이 가능하다.
 * 이는 GunItem의 파츠들이 생성자 이후 동적으로 생성되기 때문이다.
 * 
 * GunItem은 루트로 메쉬가 아닌 박스 컴포넌트를 사용한다.
 * 이는 충돌 판정을 위한 아이템 전체의 크기가 생성자 이후 결정되기 때문이다.
 */
UCLASS()
class PROJECTTR_API AGunItem : public AWieldItem
{
	GENERATED_BODY()

public:
	AGunItem();

	// 오버라이드
	virtual void BeginPlay() override;
	virtual void OnPostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnDeployerStatChanged() override;

	// 총 파츠 초기화 시 로직
	void Server_InitGunParts();
	void Client_InitGunParts();

	// 공통 로직
	void Host_InitGunParts();

	// 복구 시 캐싱해둔 클래스 정보를 기반으로 GunPartsComp들을 재생성한다
	virtual bool RestoreFromItemData(class UItemData* Data) override;

	// 1회 격발 이후 처리 로직
	void OnShotFired();

	// 현재 등록된 총기 스펙을 기준으로 설명을 생성한다
	// 여기서 생성된 설명은 아이템 액터 생성 플로우를 거치며 인벤토리 오브젝트에 바인딩된다
	void GenerateWeaponAttr();

#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		// GunItem의 경우 피직스 연산을 메쉬가 아닌 박스 콜리전 컴포넌트에서 처리한다
		DOREPLIFETIME(AGunItem, GunBoxColComponent);

		DOREPLIFETIME(AGunItem, BarrelComp);
		DOREPLIFETIME(AGunItem, GripComp);
		DOREPLIFETIME(AGunItem, MagazineComp);
		DOREPLIFETIME(AGunItem, MuzzleComp);
		DOREPLIFETIME(AGunItem, ReceiverComp);
		DOREPLIFETIME(AGunItem, SightComp);
		DOREPLIFETIME(AGunItem, StockComp);
		DOREPLIFETIME(AGunItem, GunPartsToReplicateCount);

		// 클라이언트 사이드 FX 재생에 영향을 줄 수 있는 값들은 레플리케이션한다
		DOREPLIFETIME(AGunItem, bApplyLightToMuzzleOnFire);
		DOREPLIFETIME(AGunItem, MuzzleLightColor);
		DOREPLIFETIME(AGunItem, MuzzleLightIntensity);
		DOREPLIFETIME(AGunItem, MuzzleFlashVFX);
		DOREPLIFETIME(AGunItem, ShellEjectVFX);

		DOREPLIFETIME(AGunItem, Gunshot1SFX);
		DOREPLIFETIME(AGunItem, Gunshot2SFX);
		DOREPLIFETIME(AGunItem, Gunshot3SFX);
		DOREPLIFETIME(AGunItem, Gunshot4SFX);
		DOREPLIFETIME(AGunItem, Gunshot5SFX);
		DOREPLIFETIME(AGunItem, Gunshot6SFX);
		DOREPLIFETIME(AGunItem, Gunshot7SFX);
		DOREPLIFETIME(AGunItem, Gunshot8SFX);

		DOREPLIFETIME(AGunItem, OnEmptyFireSFX);
		DOREPLIFETIME(AGunItem, OnAmmoRegainSFX);

		// 싱크가 맞아야 하는 값들을 레플레케이션한다
		DOREPLIFETIME(AGunItem, CurrAmmo);
		DOREPLIFETIME(AGunItem, AmmoPerShot);
		// TODO: 게이지 등
	}
#pragma endregion

#pragma region /** Components */
protected:
	// 총기 범위 전체에 대한 피직스 콜리전 처리를 위한 컴포넌트
	UPROPERTY(Replicated)
	TObjectPtr<class UBoxComponent> GunBoxColComponent;

protected:
	// 파츠 초기화가 전부 끝난 이후 서버사이드에서만 처리
	void InitGunBoxColComponent();

	// GunItem은 메쉬가 루트가 아니기 때문에, 메쉬의 피직스나 콜리전 관련 연산을 해제한다
	virtual void InitMeshComp(class UPrimitiveComponent* Component) override;
#pragma endregion

#pragma region /** Gun spec Getters/Setters */
// NOTE: 외부에서 총기 spec을 read할 때는 항상 Getter만을 사용해야 한다
// 인자로 전달하는 Wielder가 null이면 총기 스텟을 그대로 반환하며, 
// null이 아닐 경우 캐릭터 고유 스테이터스를 적용해 최종 스테이터스를 반환한다
// 캐릭터 스테이터스에 영향받지 않는 값들의 경우 인자가 전달되었더라도 사용되지 않을 수 있다
public:
	const FExplosionInfo GetStat_GunExplosionInfo(class AGameCharacter* Wielder) const;

	const EWeaponFireType& GetStat_GunType(class AGameCharacter* Wielder) const;
	void SetStat_GunType(const EWeaponFireType& Value) { GunType = Value; Local_ApplyGunStats(); } // NOTE: 연결된 값들도 값을 변경해주어야 함

	const EWeaponFireMode& GetStat_FireMode(class AGameCharacter* Wielder) const;
	void SetStat_FireMode(const EWeaponFireMode& Value) { FireMode = Value; Local_ApplyGunStats(); }

	const TSubclassOf<class UTRDamageType>& GetStat_DamageType(class AGameCharacter* Wielder) const;
	void SetStat_DamageType(const TSubclassOf<class UTRDamageType>& Value) { DamageType = Value; Local_ApplyGunStats(); }

	const int32 GetStat_MissileSpawnedPerShot(class AGameCharacter* Wielder) const;
	void SetStat_MissileSpawnedPerShot(int32 Value) { MissileSpawnedPerShot = Value; Local_ApplyGunStats(); }

	const bool GetStat_SetAccurateOffsetForFirstMissile(class AGameCharacter* Wielder) const;
	void SetStat_SetAccurateOffsetForFirstMissile(bool Value) { bSetAccurateOffsetForFirstMissile = Value; Local_ApplyGunStats(); }

	const float GetStat_DmgEnemyDirect(class AGameCharacter* Wielder) const;
	void SetStat_DmgEnemyDirect(float Value) { DmgEnemyDirect = Value; }

	const float GetStat_DmgAllyDirect(class AGameCharacter* Wielder) const;
	void SetStat_DmgAllyDirect(float Value) { DmgAllyDirect = Value; }

	const float GetStat_SelfDmgOnCharacterHit(class AGameCharacter* Wielder) const;
	void SetStat_SelfDmgOnCharacterHit(float Value) { SelfDmgOnCharacterHit = Value; }

	const float GetStat_SelfDmgOnCharacterMissedHitscan(class AGameCharacter* Wielder) const;
	void SetStat_SelfDmgOnCharacterMissedHitscan(float Value) { SelfDmgOnCharacterMissedHitscan = Value; }

	const float GetStat_SelfDmgOnCharacterKilled(class AGameCharacter* Wielder) const;
	void SetStat_SelfDmgOnCharacterKilled(float Value) { SelfDmgOnCharacterKilled = Value; }

	const float GetStat_DmgMultDistClose(class AGameCharacter* Wielder) const;
	void SetStat_DmgMultDistClose(float Value) { DmgMultDistClose = Value; }

	const float GetStat_DmgMultDistFar(class AGameCharacter* Wielder) const;
	void SetStat_DmgMultDistFar(float Value) { DmgMultDistFar = Value; }

	const float GetStat_DmgMultOnAirshot(class AGameCharacter* Wielder) const;
	void SetStat_DmgMultOnAirshot(float Value) { DmgMultOnAirshot = Value; }

	const bool GetStat_HitscanPiercePawns(class AGameCharacter* Wielder) const;
	void SetStat_HitscanPiercePawns(bool Value) { bHitscanPiercePawns = Value; }

	const float GetStat_GunExplosionRadius(class AGameCharacter* Wielder) const;
	void SetStat_GunExplosionRadius(float Value) { GunExplosionRadius = Value; }

	const float GetStat_GunExplosionDamage(class AGameCharacter* Wielder) const;
	void SetStat_GunExplosionDamage(float Value) { GunExplosionDamage = Value; }

	const float GetStat_GunMinExplosionMultiplier(class AGameCharacter* Wielder) const;
	void SetStat_GunMinExplosionMultiplier(float Value) { GunMinExplosionMultiplier = Value; }

	const float GetStat_GunDmgMultOnExplInstigator(class AGameCharacter* Wielder) const;
	void SetStat_GunDmgMultOnExplInstigator(float Value) { GunDmgMultOnExplInstigator = Value; }

	const float GetStat_GunExplImpactStrength(class AGameCharacter* Wielder) const;
	void SetStat_GunExplImpactStrength(float Value) { GunExplImpactStrength = Value; }

	const bool GetStat_ExplodeOnHit(class AGameCharacter* Wielder) const;
	void SetStat_ExplodeOnHit(bool Value) { bExplodeOnHit = Value; }

	const float GetStat_DmgMultOnHead(class AGameCharacter* Wielder) const;
	void SetStat_DmgMultOnHead(float Value) { DmgMultOnHead = Value; }

	const float GetStat_DmgDistFallOffMult(class AGameCharacter* Wielder) const;
	void SetStat_DmgDistFallOffMult(float Value) { DmgDistFallOffMult = Value; }

	const bool GetStat_HasDmgDistFallOff(class AGameCharacter* Wielder) const;
	void SetStat_HasDmgDistFallOff(bool Value) { bHasDmgDistFallOff = Value; }

	const bool GetStat_ApplyStatEffToEnemyOnHit(class AGameCharacter* Wielder) const;
	void SetStat_ApplyStatEffToEnemyOnHit(bool Value) { bApplyStatEffToEnemyOnHit = Value; Local_ApplyGunStats(); }

	const bool GetStat_ApplyStatEffToAllyOnHit(class AGameCharacter* Wielder) const;
	void SetStat_ApplyStatEffToAllyOnHit(bool Value) { bApplyStatEffToAllyOnHit = Value; Local_ApplyGunStats(); }

	const TArray<FStatEffectGenInfo>& GetStat_StatEffsToEnemyWhenHit(class AGameCharacter* Wielder) const;
	void SetStat_StatEffsToEnemyWhenHit(const TArray<FStatEffectGenInfo>& Value) { StatEffsToEnemyWhenHit = Value; Local_ApplyGunStats(); }
	void AddStat_StatEffToEnemyWhenHit(const TArray<FStatEffectGenInfo>& Value);

	const TArray<FStatEffectGenInfo>& GetStat_StatEffsToAllyWhenHit(class AGameCharacter* Wielder) const;
	void SetStat_StatEffsToAllyWhenHit(const TArray<FStatEffectGenInfo>& Value) { StatEffsToAllyWhenHit = Value; Local_ApplyGunStats(); }
	void AddStat_StatEffToAllyWhenHit(const TArray<FStatEffectGenInfo>& Value);

	const float GetStat_FireInterval(class AGameCharacter* Wielder) const;
	void SetStat_FireInterval(float Value) { FireInterval = Value; }

	const float GetStat_RecoilOffsetRange(class AGameCharacter* Wielder) const;
	void SetStat_RecoilOffsetRange(float Value) { RecoilOffsetRange = Value; }

	const int32 GetStat_MaxAmmo(class AGameCharacter* Wielder) const;
	void SetStat_MaxAmmo(int32 Value) { MaxAmmo = Value; }

	const int32 GetStat_AmmoPerShot(class AGameCharacter* Wielder) const;
	void SetStat_AmmoPerShot(int32 Value) { AmmoPerShot = Value; }

	const bool GetStat_ApplyImpactOnHit(class AGameCharacter* Wielder) const;
	void SetStat_ApplyImpactOnHit(bool Value) { bApplyImpactOnHit = Value; }

	const float GetStat_MissileMass(class AGameCharacter* Wielder) const;
	void SetStat_MissileMass(float Value) { MissileMass = Value; }

	const TSubclassOf<class ABaseProjectile>& GetStat_ProjectileClass(class AGameCharacter* Wielder) const;
	void SetStat_ProjectileClass(const TSubclassOf<class ABaseProjectile>& Value) { ProjectileClass = Value; Local_ApplyGunStats(); }

	const TSubclassOf<class ABaseProjectile>& GetStat_DefaultProjectileClass(class AGameCharacter* Wielder) const;
	// No setter required

	const float GetStat_ProjInitialSpeed(class AGameCharacter* Wielder) const;
	void SetStat_ProjInitialSpeed(float Value) { ProjInitialSpeed = Value; }

	const float GetStat_ProjMaxSpeed(class AGameCharacter* Wielder) const;
	void SetStat_ProjMaxSpeed(float Value) { ProjMaxSpeed = Value; }

	const bool GetStat_ProjRotationFollowsVelocity(class AGameCharacter* Wielder) const;
	void SetStat_ProjRotationFollowsVelocity(bool Value) { bProjRotationFollowsVelocity = Value; }

	const bool GetStat_ProjRotationRemainsVertical(class AGameCharacter* Wielder) const;
	void SetStat_ProjRotationRemainsVertical(bool Value) { bProjRotationRemainsVertical = Value; }

	const bool GetStat_ProjShouldBounce(class AGameCharacter* Wielder) const;
	void SetStat_ProjShouldBounce(bool Value) { bProjShouldBounce = Value; }

	const float GetStat_ProjBounciness(class AGameCharacter* Wielder) const;
	void SetStat_ProjBounciness(float Value) { ProjBounciness = Value; }

	const bool GetStat_ProjBounceOffObjIndefinitely(class AGameCharacter* Wielder) const;
	void SetStat_ProjBounceOffObjIndefinitely(bool Value) { bProjBounceOffObjIndefinitely = Value; }

	const int32 GetStat_ProjDestroyOnHitCount(class AGameCharacter* Wielder) const;
	void SetStat_ProjDestroyOnHitCount(int32 Value) { ProjDestroyOnHitCount = Value; }

	const bool GetStat_ProjPiercePawns(class AGameCharacter* Wielder) const;
	void SetStat_ProjPiercePawns(bool Value) { bProjPiercePawns = Value; }

	const float GetStat_ProjGravityScale(class AGameCharacter* Wielder) const;
	void SetStat_ProjGravityScale(float Value) { ProjGravityScale = Value; }
#pragma endregion

#pragma region /** Gun spec */
/*** 발사 로직 ***/
// NOTE: 발사를 제외한 각종 패시브 효과는 Status effect를 사용해 처리한다
protected:
/* 무기 타입 */
	// 공격 타입
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EWeaponFireType GunType = EWeaponFireType::WFT_PROJECTILE;

	// 발사 모드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EWeaponFireMode FireMode = EWeaponFireMode::WFM_AUTO;

	// 데미지 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	TSubclassOf<class UTRDamageType> DamageType;

/* 다중 발사 */
	// 단일 격발을 통해 생성되는 발사물(총알/투사체)의 수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MissileSpawnedPerShot = 1;

	// 여러 발사물이 생성될 경우 최초 생성된 발사물은 반동을 적용하지 않고 머즐 위치 그대로 정확히 발사되게 할지 여부
	// 참일 경우 첫번째 발사물은 총알 간 탄퍼짐, 격발 당 탄퍼짐 모두 무시하고 정확한 방향으로 날아간다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetAccurateOffsetForFirstMissile = true;

/* 적중 */
	// 적 직접 적중 시 부여 데미지 / 회복량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DmgEnemyDirect = 0.0f;

	// 아군 직접 적중 시 부여 데미지 / 회복량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DmgAllyDirect = 0.0f;

	// 캐릭터 적중 시 격발자 부여 데미지 / 회복량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SelfDmgOnCharacterHit = 0.0f;

	// 캐릭터 적중 실패시 격발자 부여 데미지 / 회복량 (히트스캔에만 적용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SelfDmgOnCharacterMissedHitscan = 0.0f;

	// 캐릭터 처치 시(적대적인 경우만) 격발자 부여 데미지 / 회복량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SelfDmgOnCharacterKilled = 0.0f;

/* 거리 별 데미지 변화 */
// NOTE: 거리에 따른 falloff와는 다른 개념임
// NOTE: 근,원거리의 기준은 매크로로 정의
	// 적중 시점의 거리가 근거리 혹은 그 안일 경우 데미지 배율 조정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DmgMultDistClose = 1.0f;

	// 적중 시점의 거리가 원거리 혹은 그 바깥일 경우 데미지 조정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DmgMultDistFar = 1.0f;

/* 에어샷 */
	// 에어샷 성공 시 데미지 배율
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DmgMultOnAirshot = 1.0f;

/* 히트스캔 관통 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHitscanPiercePawns = false;

/* 폭파 */
	// 최대 폭발 범위
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GunExplosionRadius = 0.0f;

	// 폭발 시 데미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GunExplosionDamage = 0.0f;

	// 폭발 피해 증감폭
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GunMinExplosionMultiplier = 0.0f;

	// 폭발 생성자에게의 데미지 배율 (자가피해)
	// 생성자가 플레이어일 경우 팀원에게도 동일한 값을 사용한다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GunDmgMultOnExplInstigator = 1.0f;

	// 폭발 중심점의 물리 충격량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GunExplImpactStrength = 0.0f;

	// 피격 시 폭파 판정 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bExplodeOnHit = false;

/* 피격 지점 배율 */
// NOTE: 기본 헤드샷, 바디샷 배율에 추가로 곱해진다
	// 헤드샷 배율
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DmgMultOnHead = 1.0f;

/* 기타 데미지 조정 */
	// 거리 당 데미지 감소폭
	// 거리가 MAX FALLOFF DIST 이상일 때 데미지에 곱하는 값
	// 일반적으로 멀수록 데미지가 감소하므로 1 미만이지만, 역으로 1을 초과하게 만들 수 있다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DmgDistFallOffMult = 1.0f;

	// 거리 당 데미지 감소를 사용할지 여부
	// 이 값이 false면 데미지 거리에 무관하게 무조건 1.0이 곱해짐
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasDmgDistFallOff = true;

/* 발사 속도 */
	// 연사 속도
	// 한 발을 발사하고 다음 발을 발사하기까지의 딜레이 (sec)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float FireInterval = 1.0f;

/* 탄 퍼짐 */
	// 1m 거리에서의 탄퍼짐 범위 (cm)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RecoilOffsetRange = 0.0f;

/* 장탄 */
	// 최대 보유 탄약량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MaxAmmo = 200;

	// 매 회 발사 당 소비 탄약량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AmmoPerShot)
	int32 AmmoPerShot = 1;

/* 충격량 배수 */
	// 적 적중 시 충격량 적용 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bApplyImpactOnHit = true;

	// 발사물의 충돌시 질량
	// 히트스캔일 경우 총알 한발 (Ray 1개)의 질량으로 취급한다
	// 프로젝타일일 경우 해당 프로젝타일의 물리 질량과는 별도의 값으로 사용된다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MissileMass = 0.0f;

/*** 투사체 ***/
protected:
/* 투사체 종류 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<class ABaseProjectile> ProjectileClass = nullptr;

	// 기본값
	TSubclassOf<class ABaseProjectile> DefaultProjectileClass = nullptr;

/* 투사체 속도 */
	// 투사체 초기 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ProjInitialSpeed = 0.0f;

	// 투사체 최고 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ProjMaxSpeed = 0.0f;

/* 투사체 회전 */
	// 투사체가 속도의 방향으로 틱마다 회전을 조정하는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjRotationFollowsVelocity = true;

	// 투사체가 회전 시 Yaw만 조정하는지 여부 (좌우 회전만 적용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjRotationRemainsVertical = true;

/* 투사체 바운스 */
	// 투사체의 바운스 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjShouldBounce = false;

	// 투사체의 바운스 정도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ProjBounciness = 0.0f;

	// 투사체 물체에 한해 영구적 바운스 가능 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjBounceOffObjIndefinitely = false;

/* 투사체 파괴 조건 */
	// 투사체가 몇 번 충돌해야 파괴되는지 지정
	// 바운스하지 않는 경우 1로 취급
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ProjDestroyOnHitCount = 1;

/* 투사체 관통여부 */
// NOTE: 뚫을 경우에도 Hit가 발생하기 때문에 파괴 조건을 적절한 값(높은값)으로 설정해야 함
	// 폰을 뚫고 진행하는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjPiercePawns = false;

/* 투사체 중력 */
	// 중력 적용 배율
	// 월드 중력의 몇배 중력을 받을지 지정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ProjGravityScale = 0.0f;

/*** 부여 효과 ***/
protected:
/* 적중 */
	// 적중 시 적에게 스테이터스 이펙트를 적용할 지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bApplyStatEffToEnemyOnHit = false;

	// 적중 시 적에게 적용할 스테이터스 이펙트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FStatEffectGenInfo> StatEffsToEnemyWhenHit;

	// 적중 시 아군에게 스테이터스 이펙트를 적용할 지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bApplyStatEffToAllyOnHit = false;

	// 적중 시 아군에게 적용할 스테이터스 이펙트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FStatEffectGenInfo> StatEffsToAllyWhenHit;

protected:
	UFUNCTION()
	void OnRep_AmmoPerShot();
#pragma endregion

#pragma region /** Gun status */
// NOTE: 총기 사용 상태에 따라 변하는 값들
// 실수 카운터의 경우 [0, 1] 값을 사용한다
// 하나의 무기에 정수, 실수 카운터는 각각 최대 한 개만 사용이 가능하다
public:
	// 현재 보유 탄약량
	UPROPERTY(ReplicatedUsing = OnRep_CurrAmmo, VisibleAnywhere, BlueprintReadOnly)
	int32 CurrAmmo = 0;

	// 값이 빠르게 변화할 때 클라이언트에서 부드러운 동기화를 위해 추측하는 값
	int32 Client_PredictedCurrAmmo = 0;
	int32 Client_PredictedAmmoPerShot = 1;

	// 정수 카운터 사용 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bUseIntCounter = false; // TODO

	// 게이지 사용 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bUseGauge = false; // TODO

	// 정수 카운터 값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 GunCounter = 0; // TODO

	// 게이지 값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GunGauge = GunConst::GUN_MIN_GAUGE; // TODO

	// TODO: 카운터/게이지 증감 조건

public:
	// 발사가 가능한지 여부 (PrimaryComponent만 발사 역할을 수행한다)
	// 컴포넌트가 등록되어있지 않다면 false를 반환한다
	// NOTE: 이 함수는 클라이언트에서 FX 재생 전 확인을 위해 호출할 수 있다
	UFUNCTION(BlueprintCallable)
	bool Host_CanFireShot();

	// 장탄수 Getter, Setter
	void Server_SetCurrAmmo(int32 Value);
	FORCEINLINE int32 Host_GetCurrAmmo() const { return CurrAmmo; }

protected:
	UFUNCTION()
	void OnRep_CurrAmmo();
	void Local_OnCurrAmmoUpdated();
#pragma endregion

#pragma region /** Parts */
public:
	// 파츠 스텟 재설정 (기존 값을 덮어씀)
	// 파츠가 같을 땐 스테이터스가 같음이 항상 보장되어야 하며, 파츠 내에서 별도의 난수를 사용해서는 안된다
	void InitGunStats();

	// 현재 설정된 스텟을 유효 범위 내로 clamping함
	void ValidateStats();

	// 설정된 스텟을 실제 총기 컴포넌트에 반영함
	// 호스트간 레플리케이션 되는 값들은 컴포넌트 내의 값이므로 액터 프로퍼티를 컴포넌트 프로퍼티에 바인딩하는 작업을 여기서 처리한다
	// 꼭 레플리케이션 되는 값이 아니더라도 컴포넌트 단에서 값이 변경될 수 있는 값들(Projectile 액터 정보 등)도 그 초기값을 여기서 바인딩한다
	UFUNCTION()
	void Local_ApplyGunStats();

	/* Getters */
	class UGunPartComponent* GetBarrel() const { return BarrelComp; }
	class UGunPartComponent* GetGrip() const { return GripComp; }
	class UGunPartComponent* GetMagazine() const { return MagazineComp; }
	class UGunPartComponent* GetMuzzle() const { return MuzzleComp; }
	class UGunPartComponent* GetReceiver() const { return ReceiverComp; }
	class UGunPartComponent* GetSight() const { return SightComp; }
	class UGunPartComponent* GetStock() const { return StockComp; }

	TArray<UGunPartComponent*> GetGunParts();

	/* Setters */
	void SetBarrel(class UGunPartComponent* Comp) { BarrelComp = Comp; }
	void SetGrip(class UGunPartComponent* Comp) { GripComp = Comp; }
	void SetMagazine(class UGunPartComponent* Comp) { MagazineComp = Comp; }
	void SetMuzzle(class UGunPartComponent* Comp) { MuzzleComp = Comp; }
	void SetReceiver(class UGunPartComponent* Comp) { ReceiverComp = Comp; }
	void SetSight(class UGunPartComponent* Comp) { SightComp = Comp; }
	void SetStock(class UGunPartComponent* Comp) { StockComp = Comp; }

protected:
	// 아이템 메쉬 컴포넌트를 Receiver로 설정한다
	void SetMeshToReceiver();

	// 주어진 컴포넌트의 메쉬에 해당 소켓명들이 전부 있는지 확인하고 하나라도 없다면 false를 반환한다
	bool CheckComponentMeshHasSockets(UGunPartComponent* Comp, TArray<FName> SocketNames);

	// 루트(리시버)를 제외한 파츠들의 메시를 초기화한다
	void InitGunPartMeshComp(UPrimitiveComponent* Mesh);

	// 모든 파츠들에 대해 초기화 작업을 처리한다
	void InitializeGunPartMesh();

	// 리치 컴포넌트의 크기를 동적으로 현재 메쉬에 맞게 수정한다
	virtual void ResizeReachCompToMatchItem() override;

	UFUNCTION()
	void OnRep_GunPartComp();

	// 루트와 그립 컴포넌트의 상대적 거리차에 따라 어태치 지점을 변경한다
	// 어태치 각도는 상수값으로 고정한다
	void SetRelativeAttachTransform();

	// 이 총기의 파츠를 포함한 전체 물리적 크기와 유사한 값을 반환한다
	virtual FVector GetEstimatedItemSize() override;

protected:
	// 총기 파츠 컴포넌트
	// GunPartComponent 내의 프로퍼티에 대한 클라이언트 레플리케이션은 가급적 피하는 게 권장되며,
	// 최대한 Server authoritative한 구조로 데이터를 관리하는 게 권장된다
	// 이는 만약 클라이언트로 GunPartComponent 내의 어떤 값을 변경한 결과를 전달하려 할 경우,
	// 클라이언트의 GunItem에서 모든 GunPart가 수신되었음이 보장되어야 값을 변경하는게 가능해지기 때문에 구조가 복잡해지기 때문이다
	// 만약 꼭 필요한 상황이라면 Register - Process 구조로 작업을 대기시킨 후 bClient_AllPartsReplicated가 true가 될 때 처리하면 된다
	UPROPERTY(ReplicatedUsing = OnRep_GunPartComp)
	UGunPartComponent* ReceiverComp = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_GunPartComp)
	UGunPartComponent* BarrelComp = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_GunPartComp)
	UGunPartComponent* GripComp = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_GunPartComp)
	UGunPartComponent* MagazineComp = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_GunPartComp)
	UGunPartComponent* MuzzleComp = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_GunPartComp)
	UGunPartComponent* SightComp = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_GunPartComp)
	UGunPartComponent* StockComp = nullptr;

	// 클라이언트 전용; 모든 파츠의 수신이 완료되었을 경우 true로 설정한다
	bool bClient_AllPartsReplicated = false;

public:
	UPROPERTY(ReplicatedUsing = OnRep_GunPartComp)
	int32 GunPartsToReplicateCount = INT32_MAX;
#pragma endregion

#pragma region /** Generation */
public:
	// 현재 설정된 멤버들을 기반으로 파츠 메쉬 트리를 생성한다
	void ConstructMeshTree();

protected:
	// Parent에 총기 파츠 메쉬를 부착한다
	bool AttachToPart(UGunPartComponent* Parent, UGunPartComponent* Comp, FName SocketName = NAME_None);

	// 이 아이템의 크기에 맞게 자동으로 bShouldHoldWithBothArms 값을 설정한다
	virtual void SetShouldHoldWithBothArmsBySize();

protected:
	// Y 길이 추정치가 이 길이를 넘으면 양손 파지한다
	const float YLenLimitToHoldWithBothArms = 26.0f;
#pragma endregion

#pragma region /** Visuals / VFX */
public:
	// 이 총기에 속한 모든 메쉬의 레플리케이션이 완료되는 대로 처리를 반영한다
	virtual void RegisterVisibility(bool bVisibility);

	// 이 총기에 속한 모든 메쉬의 렌더링 여부를 설정한다
	virtual void SetItemVisibility(bool bVisibility) override;

	// 머즐 플래시 VFX를 재생한다
	void SpawnMuzzleFlashVFX();

	// 머즐 플래시 라이트를 지정된 시간만큼 키고 그 후 끈다
	// 이미 라이트가 켜져있을 경우 해당 호출은 무시된다
	void FlashMuzzleLight(float Duration);

	// 라이트 해제
	UFUNCTION()
	void DisableMuzzleLight();

	// 탄피 배출 VFX를 재생한다
	void SpawnShellEjectVFX();

	// 아이템의 현재 정보를 기반으로 적절한 반동 프로퍼티를 계산해 갱신한다
	// 이 함수는 서버와 클라 모두에서, 반동 애니메이션과 관련된 값이 변경될 때 호출해주어야 한다
	// 단, 갱신된 값은 레플리케이션 속도와 순서에 따라 여기서 바로 캐릭터 RecoilComponent에 반영될 수도 있고, Deploy 시에 반영될 수도 있다
	// 이 함수는 동일 조건에 대해 여러번 호출될 것을 상정하고 제작되었기 때문에 조건이 변하지 않았다면 여러번 호출해도 최종결과가 달라져서는 안된다
	void RefreshRecoilAnim();

	// 현재 총기의 총구를 구하기 위해 사용 가능한 메쉬와 소켓 정보를 반환한다
	// 기본적으로 배럴 끝의 MuzzleSocket을 사용하지만, 배럴이 없는 경우 리시버의 배럴 소켓을 반환한다
	TPair<USceneComponent*, FName> GetMuzzleCompAndSock();

protected:
	// 라이트 컴포넌트를 총구 메쉬에 부착한다
	// 적합한 총구를 찾지 못했을 경우 기본값을 사용한다
	void InitMuzzleLightComp();

	// 나이아가라 컴포넌트의 초기 설정을 처리한다
	void InitVFXComp();

	// 나이아가라 에셋이 변경되거나 초기화되었을 때 호출
	UFUNCTION()
	void OnRep_VFXAssetChanged();

protected:
	// 총기 격발 시 VFX를 위한 포인트 라이트
	TObjectPtr<class UPointLightComponent> MuzzleFlashLightComp = nullptr;

	// 라이팅 토글 시간 측정을 위해 사용하는 타이머
	FTimerHandle MuzzleFlashTimerHandle;

public:
/* 에셋 리소스 */
	// FX 리소스
	UPROPERTY(EditDefaultsOnly)
	class UFxConfig* FxConfig = nullptr;

	// 카메라 셰이크 리소스
	UPROPERTY(EditDefaultsOnly)
	class UCamShakeConfig* CamShakeConfig = nullptr;

	// 투사체 리소스
	UPROPERTY(EditDefaultsOnly)
	class UProjectileConfig* ProjectileConfig = nullptr;

	// 총알 리소스
	UPROPERTY(EditDefaultsOnly)
	class UBulletConfig* BulletConfig = nullptr;

/* 불릿 트레이서 종류 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	struct FBulletData BulletStruct;

	// 초기화 과정에서 BulletStruct에 유효한 값이 들어왔는지 판별하는 플래그 (nullcheck)
	bool bBulletStructValid = false;

	// 유효하지 않을 경우 사용할 기본값
	struct FBulletData DefaultBulletStruct;

/* 머즐 플래시 VFX */
	UPROPERTY(ReplicatedUsing = OnRep_VFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	class UNiagaraSystem* MuzzleFlashVFX = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraComponent* MuzzleFlashComp = nullptr;

/* 탄피 배출 VFX */
	UPROPERTY(ReplicatedUsing = OnRep_VFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	class UNiagaraSystem* ShellEjectVFX = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraComponent* ShellEjectComp = nullptr;

/* 폭발 VFX */
	// NOTE: 이 값들은 총기에 직접 사용되지 않고, 필요 시 Explosion 객체로 전달된다
	// 또한 Explosion 액터 단위에서 레플리케이션 되므로 여기서 미리 레플리케이션 할 필요가 없다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ENiagaraReference ExplosionVFXEnum = ENiagaraReference::ENR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ExplosionVFXRadiusConstant = 1.0f;

/* 카메라 셰이크 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<class UTRCameraShake> CameraShakeClass = nullptr;

/* 라이팅 */
	// 총기 격발 시 라이트를 토글할지 여부
	// 라이트 없이 VFX만 처리하는 것도 가능하다
	UPROPERTY(Replicated)
	bool bApplyLightToMuzzleOnFire = false;

	// 격발 시 적용할 라이트 색상
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	FColor MuzzleLightColor = FColor(0, 0, 0);

	// 격발 시 적용할 라이트 세기
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	float MuzzleLightIntensity = 0.0f;
#pragma endregion

#pragma region /** Audio / SFX */
public:
	// 오디오 리소스
	UPROPERTY(EditDefaultsOnly)
	class UAudioConfig* AudioConfig = nullptr;

	/* 격발음 */
	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* Gunshot1SFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* Gunshot2SFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* Gunshot3SFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* Gunshot4SFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* Gunshot5SFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* Gunshot6SFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* Gunshot7SFX = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* Gunshot8SFX = nullptr;

	/* 격발 실패음 */
	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* OnEmptyFireSFX = nullptr;

	/* 장전음 */
	UPROPERTY(ReplicatedUsing = OnRep_SFXAssetChanged, VisibleAnywhere, BlueprintReadOnly)
	USoundCue* OnAmmoRegainSFX = nullptr;

protected:
	TArray<USoundCue*> Local_GunshotSFXCandidates;

	// 사운드 초기 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundAttenuation* GunSoundAttenuation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundConcurrency* GunSoundConcurrency = nullptr;

public:
	// 사운드 에셋이 변경되거나 초기화되었을 때 호출
	UFUNCTION()
	void OnRep_SFXAssetChanged();

	// 총성을 랜덤으로 선택해 재생한다
	void Local_PlayGunshotSFX(bool bIsAmmoEmpty, float Volume = 1.0f, float Pitch = 1.0f);

protected:
	// 오디오 컴포넌트의 초기 설정을 처리한다
	void InitSFXComp();

	// 현재 바인딩된 총성 SFX들 중 랜덤으로 하나를 선택해 반환한다
	// nullptr인 값은 후보로 포함하지 않지만, 모든 후보가 nullptr인 경우 nullptr를 반환한다
	USoundCue* Local_GetRandomGunshotSFX();

	// 현재 바인딩된 탄약이 빌 때의 격발음을 선택해 반환한다
	USoundCue* Local_GetRandomEmptyFireSFX();
#pragma endregion
};
