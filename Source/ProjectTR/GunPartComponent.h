// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TREnums.h"
#include "FxConfig.h"
#include "AudioConfig.h"
#include "CamShakeConfig.h"
#include "ProjectileConfig.h"
#include "BulletConfig.h"
#include "StatusEffect.h"
#include "GunPartComponent.generated.h"

UCLASS( Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTTR_API UGunPartComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UGunPartComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region /** Random generation */
// 필요 시 하위 클래스에서 함수를 재정의하는 형태로 수정할 수 있다
// 인스턴스 생성 전에 액세스하는 값들이기 때문에 static을 사용한다
public:
	// 파츠 티어
	// 1, 2, 3 순으로 숫자가 높아질 수록 고등급을 의미한다
	static const int GetTier() { return 1; }

	// 파츠 생성 확률 (weight)
	// 값이 클 수록 더 생성되기 쉬움을 의미한다
	static const float GetSpawnRate() { return 1.0f; }

	// 이 파츠의 타입을 반환한다
	// 하위 클래스에서 반드시 재정의해야 한다
	static const EGunPartType GetPartType() { return EGunPartType::EGT_Undefined; }

public:
	// 이 파츠가 부착된 총기에 추가 파츠가 생성될 확률의 추가치를 나타낸다
	// 각 파츠들의 추가치들의 합만큼 추가 파츠가 생성될 확률을 가진다
	// NOTE: 이때 해당하는 추가 파츠 생성 이후 생성되는 파츠들의 경우 이 값이 실제 생성 확률에 영향을 주지 않을 수 있다
	// 따라서 파츠 생성 순서를 제대로 확인하는 것이 중요하다
	virtual const float GetMuzzleGenChanceDelta() const { return 0.0f; }
	virtual const float GetSightGenChanceDelta() const { return 0.0f; }
	virtual const float GetStockGenChanceDelta() const { return 0.0f; }
#pragma endregion

#pragma region /** Mesh */
public:
	// 사용할 메쉬를 기반으로 새 Mesh Component를 만들고 초기화한다
	// Static과 Skeletal mesh중 하나만 사용한다
	// 둘 다 nullptr가 아닐 경우 Static을 우선해서 사용한다
	// 반드시 생성자에서만 사용한다
	void SetupMeshComp(UStaticMesh* Static, USkeletalMesh* Skeletal);

	// 메쉬 컴포넌트를 반환한다
	UMeshComponent* GetMeshComp();

	// 현재 메쉬 컴포넌트의 타입을 반환한다
	EGunPartMeshType GetMeshCompType();

	// 메쉬 오프셋을 반환한다
	const FVector& GetMeshOffset();

protected:
	// 이 파츠에 할당된 메쉬 컴포넌트
	// Static 혹은 Skeletal mesh component를 사용한다
	UMeshComponent* MeshComponent = nullptr;

protected:
	// 둘 중 하나만 유효한 값을 가지도록 설정한다
	// 이 값은 일반적인 상황에선 BeginPlay 이후 변경되지 않는다
	// 모든 메쉬에 대한 로직은 이 변수들 대신 MeshComponent로 처리한다
	// 특정 클래스에 대한 로직이 필요한 경우 GetMeshCompType()을 사용해 타입을 받아와 캐스팅한다
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	UStaticMesh* StaticMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	USkeletalMesh* SkeletalMesh = nullptr;

	// 메쉬 오프셋
	// 메쉬 부착 시의 오프셋을 의미한다
	FVector MeshOffset = { 0, 0, 0 };
#pragma endregion

#pragma region /** Logic */
public:
	// 총기에 이 파츠의 효과를 적용한다
	// 패스를 구분지어 순서대로 처리한다
	// 일반적인 경우 Delta(상수값 증감)는 패스1에, Mult(배율 적용)는 패스2에 처리한다
	// 완전한 오버라이드(상수값 지정)가 일어나야 하는 경우 패스3에서 처리한다
	virtual void StatPass1(class AGunItem* Gun);
	virtual void StatPass2(class AGunItem* Gun);
	virtual void StatPass3(class AGunItem* Gun);
#pragma endregion

#pragma region /** Gun spec */
/*** 발사 로직 ***/
/* 무기 타입 */
	// NOTE:
	// 어떤 값은 Delta, Multiplier 같이 
	// 하나의 총기를 구성하는 파츠 내에서 오버라이드 요청이 여러 번 일어나지 않도록 
	// 총기 파츠 종류별로 오버라이드가 가능한 요소에 제약을 두어야 한다

	// 공격 타입
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideGunType = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EWeaponFireType GunTypeValue = EWeaponFireType::WFT_PROJECTILE;

	// 발사 모드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideFireMode = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EWeaponFireMode FireModeValue = EWeaponFireMode::WFM_AUTO;

	// 데미지 타입
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideDamageType = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<class UTRDamageType> DamageTypeValue = nullptr;

	/* 총알 종류 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetBulletFromEnum = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EBulletReference BulletEnum = EBulletReference::EBU_NULL;

/* 다중 발사 */
	// 단일 격발 발사물 수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 DeltaMissileSpawnedPerShot = 0;

	// 여러 발사물이 생성될 경우 최초 생성된 발사물은 반동을 적용하지 않고 머즐 위치 그대로 정확히 발사되게 할지 여부
	// NOTE: 각 발사물간의 탄퍼짐 정도는 기본 단일격발 시 사용되는 탄퍼짐 값을 그대로 사용한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bOverrideSetAccurateOffsetForFirstMissile = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetAccurateOffsetForFirstMissileValue = true;

/* 적중 */
	// 적 직접 적중 시 부여 데미지 / 회복량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaDmgEnemyDirect = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MultDmgEnemyDirect = 1.0f;

	// 아군 직접 적중 시 부여 데미지 / 회복량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaDmgAllyDirect = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MultDmgAllyDirect = 1.0f;

	// 격발자의 캐릭터 적중 시 데미지 / 회복량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaSelfDmgOnCharacterHit = 0.0f;

	// 격발자의 캐릭터 적중 실패 시 데미지 / 회복량 (히트스캔에만 적용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaSelfDmgOnCharacterMissedHitscan = 0.0f;

	// 격발자의 캐릭터 처치 시 데미지 / 회복량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaSelfDmgOnCharacterKilled = 0.0f;

/* 거리 별 데미지 변화 */
	// 근거리 적중 시 추가 데미지 배율 조정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaDmgMultDistClose = 0.0f;

	// 원거리 적중 시 추가 데미지 배율 조정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaDmgMultDistFar = 0.0f;

/* 에어샷 */
	// 에어샷 적중 시 추가 데미지 배율 조정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaDmgMultOnAirshot = 0.0f;

/* 관통 */
	// 히트스캔 관통
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideHitscanPiercePawns = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHitscanPiercePawnsValue = false;

/* 폭파 */
	// 폭발 범위
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaExplosionRadius = 0.0f;

	// 폭발 시 부여 데미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaExplosionDmg = 0.0f;

	// 폭발 피해 증감폭
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideMinExplosionMultiplier = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MinExplosionMultiplierValue = 0.0f;

	// 생성자 데미지 배율
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaDmgMultOnExplInstigator = 0.0f;

	// 폭발 중심점 물리 충격량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaBaseImpactStrength = 0.0f;

	// 피격 시 폭파 판정 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideExplodeOnHit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bExplodeOnHitValue = false;

/* 폭발 VFX */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideExplosionVFXEnum = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ENiagaraReference ExplosionVFXEnumValue = ENiagaraReference::ENR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideExplVFXRadiusConstant = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ExplVFXRadiusConstantValue = 1.0f;

/* 피격 지점 배율 */
	// 헤드샷 배율
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaDmgMultOnHead = 0.0f;

/* 기타 데미지 조정 */
	// 거리 당 데미지 감소폭
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaDmgDistFallOffMult = 0.0f;

	// 거리 당 데미지 감소를 사용할지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideHasDmgDistFallOff = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasDmgDistFallOffValue = true;

/* 상태이상 부여 */
	// 적중 시 적에게 스테이터스 이펙트를 적용할 지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideApplyStatEffToEnemyOnHit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bApplyStatEffToEnemyOnHitValue = false;

	// 적중 시 적에게 적용할 스테이터스 이펙트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FStatEffectGenInfo> AddStatEffsToEnemyWhenHit;

	// 적중 시 아군에게 스테이터스 이펙트를 적용할 지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverridebApplyStatEffToAllyOnHit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bApplyStatEffToAllyOnHitValue = false;

	// 적중 시 아군에게 적용할 스테이터스 이펙트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FStatEffectGenInfo> AddStatEffsToAllyWhenHit;

/* 발사 속도 */
	// 연사 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaFireInterval = 0.0f;

/* 탄 퍼짐 */
	// 1m 거리에서의 탄퍼짐 범위 (cm)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaRecoilOffsetRange = 0.0f;

/* 장탄 */
	// 최대 보유 탄약량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int DeltaMaxAmmo = 0;

	// 매 회 발사 당 소비 탄약량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int DeltaAmmoPerShot = 0;

/* 충격량 배수 */
	// 적 적중 시 충격량 적용 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideApplyImpactOnHit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bApplyImpactOnHitValue = true;

	// 발사체의 질량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideMissileMass = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MissileMassValue = 0.0f;

/*** 발사 제외 로직 ***/
protected:
/* 최대 체력 */
	// Deploy 시 최대 체력 증가량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 DeltaMaxHealthBonusOnDeployed = 0;

/* 데미지 증감 */
	// Deploy 시 격발자의 피격 데미지 증감률
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaReceiveDmgMultOnDeployed = 0.0f;

/* 회복 증감 */
	// Deploy 시 회복(음수 데미지) 증감률
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaReceiveNegDmgMultOnDeployed = 0.0f;

/* 이동 관련 */
	// 이동 속도 증감률
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaMovementSpeedMultOnDeployed = 0.0f;

	// 점프 높이 증감률
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaJumpHeightMultOnDeployed = 0.0f;

	// 점프 횟수 증가량
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 DeltaBonusJumpCount = 0;
#pragma endregion

#pragma region /** Gun spec - Projectile */
protected:
/* 투사체 종류 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetProjectileFromEnum = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EProjectileReference ProjectileEnum = EProjectileReference::EPR_NULL;

/* 투사체 속도 */
	// 투사체 초기 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaProjInitialSpeed = 1.0f;

	// 투사체 최고 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaProjMaxSpeed = 1.0f;

/* 투사체 회전 */
	// 투사체가 속도의 방향으로 틱마다 회전을 조정하는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideProjRotationFollowsVelocity = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjRotationFollowsVelocityValue = true;

	// 투사체가 회전 시 Yaw만 조정하는지 여부 (좌우 회전만 적용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideProjRotationRemainsVertical = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjRotationRemainsVerticalValue = true;

/* 투사체 바운스 */
	// 투사체의 바운스 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideProjShouldBounce = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjShouldBounceValue = false;

	// 투사체의 바운스 정도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideProjBounciness = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ProjBouncinessValue = 0.0f;

	// 투사체의 물체 한정 영구적 바운스 허용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideProjBounceOffObjIndefinitely = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjBounceOffObjIndefinitelyValue = false;

/* 투사체 파괴 조건 */
	// 투사체가 몇 번 충돌해야 파괴되는지 지정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideProjDestroyOnHitCount = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ProjDestroyOnHitCountValue = 1;

/* 투사체 관통여부 */
	// 폰을 뚫고 진행하는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideProjPiercePawns = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProjPiercePawnsValue = false;

/* 투사체 중력 */
	// 중력 오버라이드 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideProjGravityScale = false;

	// 중력 적용 배율
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ProjGravityScaleValue = 0.0f;
#pragma endregion

#pragma region /** Gun spec - Charge */
protected:
	// 정수 카운터 사용 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideUseIntCounter = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bUseIntCounterValue = false;

	// 게이지 사용 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideUseGauge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bUseGaugeValue = false;
#pragma endregion

#pragma region /** VFX */
protected:
/* 머즐 플래시 VFX */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetMuzzleFlashVFXFromEnum = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ENiagaraReference MuzzleFlashVFXEnum = ENiagaraReference::ENR_NULL;

/* 탄피 배출 VFX */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetShellEjectVFXFromEnum = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ENiagaraReference ShellEjectVFXEnum = ENiagaraReference::ENR_NULL;

/* 카메라 셰이크 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetCameraShakeFromEnum = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECamShakeReference CameraShakeClassEnum = ECamShakeReference::ECR_NULL;

/* 격발 시 라이팅 */
	// 라이팅 사용 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideApplyLightToMuzzleOnFire = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bApplyLightToMuzzleOnFireValue = true;

	// 라이팅 색상
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideMuzzleLightColor = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FColor MuzzleLightColorValue;

	// 라이팅 세기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOverrideMuzzleLightIntensity = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MuzzleLightIntensityValue;
#pragma endregion

#pragma region /** SFX */
protected:
/* 격발 SFX */
	// 격발 시
	// 토글 시 8개 모두에 대해 오버라이드가 적용된다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetGunshotSFXFromEnum = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunshotAudioReference Gunshot1SFXEnum = EGunshotAudioReference::EGR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunshotAudioReference Gunshot2SFXEnum = EGunshotAudioReference::EGR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunshotAudioReference Gunshot3SFXEnum = EGunshotAudioReference::EGR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunshotAudioReference Gunshot4SFXEnum = EGunshotAudioReference::EGR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunshotAudioReference Gunshot5SFXEnum = EGunshotAudioReference::EGR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunshotAudioReference Gunshot6SFXEnum = EGunshotAudioReference::EGR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunshotAudioReference Gunshot7SFXEnum = EGunshotAudioReference::EGR_NULL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunshotAudioReference Gunshot8SFXEnum = EGunshotAudioReference::EGR_NULL;

	// 격발 실패 시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSetOnEmptyFireSFXFromEnum = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGunMiscAudioReference OnEmptyFireSFXEnum = EGunMiscAudioReference::EGM_NULL;
#pragma endregion
};
