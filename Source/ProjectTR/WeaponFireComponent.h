// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "ActItemComponent.h"
#include "TREnums.h"
#include "StatusEffect.h"
#include "BulletConfig.h"
#include "Net/UnrealNetwork.h"
#include "WeaponFireComponent.generated.h"

/**
 * NOTE: 
 * GunItem에 dependent한 컴포넌트로, 로직 관리의 용이함을 위해 별도의 컴포넌트로 분리함.
 * Gun와 개별적인 격발은 RangedAttackComponent를 사용할 것
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTTR_API UWeaponFireComponent : public UActItemComponent
{
	GENERATED_BODY()

protected:
	UWeaponFireComponent();

#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		
		// FX 재생에 관여하는 값들의 경우 클라이언트도 싱크가 맞아야 한다
		// FX 로직
		DOREPLIFETIME(UWeaponFireComponent, CurrFireMode);
		DOREPLIFETIME(UWeaponFireComponent, CurrGunType);
		DOREPLIFETIME(UWeaponFireComponent, CurrDamageType);
		DOREPLIFETIME(UWeaponFireComponent, CurrRapidFireInterval);
		DOREPLIFETIME(UWeaponFireComponent, CurrRecoilOffsetRange);
		DOREPLIFETIME(UWeaponFireComponent, FireMissilePerShot);
		DOREPLIFETIME(UWeaponFireComponent, bFireSetAccurateOffsetForFirstMissile);

		// VFX
		DOREPLIFETIME(UWeaponFireComponent, FireCameraShakeClass);

		DOREPLIFETIME(UWeaponFireComponent, FireBulletStruct);
		DOREPLIFETIME(UWeaponFireComponent, FireProjClass);
	}
#pragma endregion

#pragma region /** Core Logic */
public:
	// 트리거 가능한지 확인
	virtual bool Host_CanTrigger(class AGameCharacter* Invoker) override;

/* 격발 목표 지점 예상 */
protected:
	// 주어진 지점에서 방향으로 LineTrace를 계산해 처음 감지되는 사격 가능한 게임 내 타깃의 거리를 계산해 CurrVirtualTargetRange에 저장한다
	void SetupVirtualTargetDistance(const FVector& StartPos, const FVector& Direction, AActor* FireActor);

/* 단일 시행 로직 */
protected:
	// 단일 시행 격발 처리
	// 격발 시 발생하는 최소한의 액션을 정의한다
	// 오버라이드 시 반드시 부모 함수를 호출해야 한다
	UFUNCTION()
	virtual void Fire();

	// 현재 격발 로직을 할 때 필요한 값들을 구해 멤버에 저장한다
	// 반드시 미리 CurrFireActor를 지정해주어야 한다
	// NOTE: 탄퍼짐 연산은 처리하지 않으므로 별도로 계산해주어야 한다
	// NOTE: 클라이언트에서도 FX 재생에 필요한 값을 구하기 위해 호출할 수 있다
	virtual void Host_SetupFireFromMuzzle(class AGameCharacter* FireActor);

	// 주어진 기준 방향에 대해 오프셋 이내의 랜덤한 반동 위치를 반환한다
	FVector RandomRecoil(FVector Direction, float OffsetRange);

	// 주어진 기준 방향에 대해 오프셋 이내의 랜덤한 반동 위치를 반환하되,
	// ShotCount 값이 같은 경우 같은 결과를 반환한다
	FVector FixedRecoil(FVector Direction, float OffsetRange, int32 ShotCount);

	FVector2D FixedPointInArea(float SquareLen, int32 PointIdx);
	FVector2D RandPointInCircleSeeded(float CircleRadius, int32 Seed);

	// 유형 별 처리 게임 로직
	void FireHitscan();
	void FireProjectile();

private:
	// 히트스캔 1회의 로직 처리
	// 유효한 히트가 발생해 처리를 완료한 경우 true를 반환한다
	// 충돌한 개별 캐릭터의 개수만큼 out_CharactersHit값이 설정된다
	// 중요: out_HitResult는 최초 발생한 아우터 히트박스가 아닌 유효 충돌 정보를 반환한다
	bool ProcessHitscanSingle(FHitResult& out_HitResult, bool& out_bHitResultValid, int32& out_CharactersHit, const FVector& LineTraceBegin, const FVector& LineTraceEnd, const FVector& TraceDirection, uint8 RecursiveDepth, uint8 MaxRecursiveDepth, TArray<UPrimitiveComponent*>& IgnoredComponents, TArray<AActor*>& IgnoredActors, bool bIgnoreColVFX);

/* 격발 로직 */
private:
	// 모드 별 격발 로직
	void OnSafeFire();
	void OnSingleFire();
	void OnAutoFire();
	void OnBurstFire();

	// 격발 중지 로직
	void OnStopFire();

	// 연사 및 점사 격발 중지 로직
	void OnStopRapidFire();

	// 모드에 맞는 로직 실행
	void PerformFire(class AGameCharacter* Pawn, EWeaponFireMode FireMode);
	void StopFire(class AGameCharacter* Pawn, EWeaponFireMode FireMode);

/* 트리거 로직 */
public:
	// 트리거 로직
	virtual bool TriggeredByPlayer(class AFPSCharacter* PlayerPawn) override;
	virtual bool TriggeredByAI(class AGameCharacter* AIPawn) override;
	virtual bool StoppedByPlayer(class AFPSCharacter* PlayerPawn) override;
	virtual bool StoppedByAI(class AGameCharacter* AIPawn) override;

public:
	// 발사 모드
	// NOTE: 장착 해제 후 재장착 시 아이템 액터 및 하위 컴포넌트가 파괴되면서 이 값도 초기화된다
	UPROPERTY(Replicated, EditDefaultsOnly)
	EWeaponFireMode CurrFireMode = EWeaponFireMode::WFM_AUTO;

	// 화기 종류
	UPROPERTY(Replicated, EditDefaultsOnly)
	EWeaponFireType CurrGunType = EWeaponFireType::WFT_PROJECTILE;

	// 데미지 종류
	UPROPERTY(Replicated, EditDefaultsOnly)
	TSubclassOf<class UTRDamageType> CurrDamageType = nullptr;

	// 연사 속도 (sec)
	UPROPERTY(ReplicatedUsing = OnRep_RecoilAnimInfluencer, EditDefaultsOnly)
	float CurrRapidFireInterval = 0.3f;

	// 1m 거리에서의 탄퍼짐 범위 (cm); 0 이하인 경우 탄퍼짐이 없음을 의미한다
	UPROPERTY(Replicated, EditDefaultsOnly)
	float CurrRecoilOffsetRange = 2.0f;

	// Bullet 정보
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Hitscan")
	struct FBulletData FireBulletStruct;

	// 투사체 클래스
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<class ABaseProjectile> FireProjClass = nullptr;

	// 단일 격발 시행 처리 시 생성되는 탄의 수
	UPROPERTY(Replicated, EditDefaultsOnly)
	int32 FireMissilePerShot = 1;

	// 단일 격발 시 여러 탄이 생성될 경우 첫 생성 탄은 탄퍼짐 없이 발사할지 여부
	UPROPERTY(Replicated, EditDefaultsOnly)
	bool bFireSetAccurateOffsetForFirstMissile = true;

	// 적중 시 적에게 스테이터스 이펙트를 적용할 지 여부
	UPROPERTY(EditDefaultsOnly)
	bool bApplyStatEffToEnemyOnHit = false;

	// 적중 시 적에게 적용할 스테이터스 이펙트들
	UPROPERTY(EditDefaultsOnly)
	TArray<FStatEffectGenInfo> StatEffsToEnemyWhenHit;

	// 적중 시 아군에게 스테이터스 이펙트를 적용할 지 여부
	UPROPERTY(EditDefaultsOnly)
	bool bApplyStatEffToAllyOnHit = false;

	// 적중 시 아군에게 적용할 스테이터스 이펙트들
	UPROPERTY(EditDefaultsOnly)
	TArray<FStatEffectGenInfo> StatEffsToAllyWhenHit;

protected:
	// 격발 주기 관리용 타이머
	FTimerHandle RapidFireTimerHandle;

	// 연발 시 로직 델리게이트
	FTimerDelegate RapidFireDelegate;

	// 연사/점사 중인지 여부
	bool bIsRapidFiring = false;

/* 격발 캐시 */
protected:
	// 매 단일 시행 마다 사용하는 값
	// 카메라 머즐 기준
	FVector CurrCamMuzzleLocation;
	FRotator CurrCamMuzzleRotation;

	// 메쉬 머즐 기준 (총기 모델의 총구 위치)
	FVector CurrMeshMuzzleLocation;
	FRotator CurrMeshMuzzleRotation;

	// 격발자
	class AGameCharacter* CurrFireActor = nullptr;

	// 최대 유효 사거리
	const float MaxTargetRange = 100000.0f;

	// 가상 목표지점 거리
	float CurrVirtualTargetDistance = MaxTargetRange;

/* 반동 애니메이션 */
public:
	// 반동 애니메이션 정보에 영향을 주는 프로퍼티의 레플리케이션 시 클라이언트에서 호출된다
	UFUNCTION()
	void OnRep_RecoilAnimInfluencer() const;
#pragma endregion

#pragma region /** Hitscan Logic */
protected:
	// 개별 충돌에 대한 로직을 작성한다; 즉 관통 시 매 충돌마다 처리해야 하는 로직들을 정의한다
	// GunOwner 혹은 Shooter 인자가 nullptr인 경우 처리를 중단한다
	void OnPerHitscanHitboxCollision(class UHitboxComponent* HitboxComp, FVector NormalImpulse, const FHitResult& Hit, class AGunItem* GunOwner, class AGameCharacter* Shooter, bool bIgnoreColVFX);
	void OnPerHitscanObjectCollision(TWeakObjectPtr<class UPrimitiveComponent> ObjectComp, FVector NormalImpulse, const FHitResult& Hit, class AGunItem* GunOwner, class AGameCharacter* Shooter, bool bIgnoreColVFX);

	// 히트스캔 격발 매 회에 대한 로직을 작성한다; 즉 관통 여부와 무관하게 한번만 처리되는 로직들을 정의한다
	// 유효한 히트스캔 충돌이 없었다면 FirstValidHit는 nullptr가 전달된다
	void OnHitscanFired(class AGunItem* GunOwner, class AGameCharacter* Shooter, FHitResult* FirstValidHit, int32 CharactersHit);
#pragma endregion

#pragma region /* FX Logic */
/* 단일 시행 로직 */
protected:
	// 단일 Fx 재생 처리
	UFUNCTION()
	virtual void Local_Fx();

/* 격발 로직 */
private:
	// 모드 별 fx 로직
	void Local_OnSafeFx();
	void Local_OnSingleFx();
	void Local_OnAutoFx();
	void Local_OnBurstFx();

	// fx 중지 로직
	void Local_OnStopFx();

	// 연사 및 점사 fx 중지 로직
	void Local_OnStopRapidFx();

	// 모드에 맞는 fx 실행
	void Local_StartFx(class AGameCharacter* Pawn, EWeaponFireMode FireMode);
	void Local_EndFx();

/* 트리거 로직 */
public:
	// FX 트리거 로직
	virtual void Local_PlayFx(class AGameCharacter* Invoker) override;
	virtual void Local_StopFx(class AGameCharacter* Invoker) override;

/* 나이아가라 */
// 불릿 콜리전 VFX
// NOTE: 투사체 콜리전 VFX는 투사체 내 메서드에서 처리
protected:
	void Server_SpawnBulletHitObjectVFX(FVector HitLocation, FRotator HitRotation);
	void Server_SpawnBulletHitCharacterVFX(FVector HitLocation, FRotator HitRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnBulletHitObjectVFX(FVector HitLocation, FRotator HitRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnBulletHitCharacterVFX(FVector HitLocation, FRotator HitRotation);

// 불릿 트레이서 VFX
protected:
	UFUNCTION(NetMulticast, Unreliable /* 성능 위함 */)
	void Multicast_SpawnBulletTracerVFX(FVector Direction, APlayerController* InvokeHost);

	void Local_SpawnBulletTracerVFX(FVector Direction);

/* FX 변수 - 로컬 */
protected:
	// 격발 FX 주기 관리용 타이머
	FTimerHandle Local_RapidFxTimerHandle;

	// 연발 시 FX 델리게이트
	FTimerDelegate Local_RapidFxDelegate;

	// 연사/점사 FX 재생 중인지 여부
	bool bLocal_IsRapidFxPlaying = false;

/* FX 캐시 */
protected:
	class AGameCharacter* Local_CurrFxActor = nullptr;
#pragma endregion

#pragma region /** FX Resources */
public:
	// 이 WeaponFireComponent의 오너가 플레이어 캐릭터일 때 재생할 카메라 셰이크
	UPROPERTY(Replicated)
	TSubclassOf<class UTRCameraShake> FireCameraShakeClass = nullptr;

	/* VFX 토글 */
	UPROPERTY(EditDefaultsOnly)
	bool bUseMuzzleFlash_Projectile = true;

	UPROPERTY(EditDefaultsOnly)
	bool bUseMuzzleFlash_Hitscan = true;

	UPROPERTY(EditDefaultsOnly)
	bool bUseShellEjection_Projectile = true;

	UPROPERTY(EditDefaultsOnly)
	bool bUseShellEjection_Hitscan = true;

	/* SFX 설정 */
	UPROPERTY(EditDefaultsOnly)
	bool bUseGunShotAudio_Projectile = true;

	UPROPERTY(EditDefaultsOnly)
	bool bUseGunShotAudio_Hitscan = true;
#pragma endregion
};
