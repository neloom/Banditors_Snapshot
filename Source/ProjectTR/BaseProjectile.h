// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRExplosion.h"
#include "StatusEffect.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "BaseProjectile.generated.h"

UCLASS()
class PROJECTTR_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	ABaseProjectile();

	// bUseHitNormalForVFX 값에 따라 충돌 VFX 재생에 사용할 적절한 Rotation 값을 반환한다
	// bUseHitNormalForVFX가 true일 경우 충돌 지점의 Normal값을, false 일 경우 함수 호출 시점의 투사체의 진행방향의 반대 방향을 반환한다
	FRotator GetHitRotationForVFX(const FHitResult& Hit) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostNetReceiveLocationAndRotation() override;

	// AActor::Destroy 대신 이 함수를 사용해 파괴를 처리하는 것이 권장된다
	void Server_DestoryProjectile();

#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	}
#pragma endregion

#pragma region /** Client movement prediction */
private:
	FVector Client_ProjLerpTargetLocation;
	FRotator Client_ProjLerpTargetRotation;
#pragma endregion

#pragma region /** Components */
public:
	// 투사체 이동 컴포넌트
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	TObjectPtr<class UTRProjMovementComponent> ProjectileMovementComponent = nullptr;

	// 발사체 메쉬
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TObjectPtr<class UStaticMeshComponent> ProjectileMeshComponent = nullptr;
#pragma endregion

#pragma region /** Initializer */
protected:
	// 투사체 이동 컴포넌트의 세부 수치 초기화; 생성자에서 호출할 용도로 제작되었음
	virtual void InitProjectileMovement(class UTRProjMovementComponent* Component);

	// 메쉬 세부 수치 초기화; 생성자에서 호출할 용도로 제작되었음
	virtual void InitProjectileMesh(UStaticMeshComponent* Component);
#pragma endregion

#pragma region /** Collision */
protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
#pragma endregion

#pragma region /** Gameplay */
// 모든 값은 서버사이드에서 처리된다. 
// 게임플레이와 관련된 다수의 값들이 ProjectileMovementComponent에서 관리되며, 여기서는 그 외의 값들을 관리한다

/* Explosion */
public:
	// 투사체가 충돌했을 때 소환할 폭발물 타입
	// 이 값은 bShouldOverrideExplosiveinfo가 참인 경우 nullptr일 수 있다
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATRExplosion> SpawnExplosiveClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	bool bShouldSpawnExplOnCollision = false;

	// 폭발 정보를 오버라이드 할 지 여부
	// 이 값이 참일 경우 SpawnExplosiveClass는 무시되며, 해당 info를 기반으로 새 폭발 인스턴스가 생성된다
	UPROPERTY(EditDefaultsOnly)
	bool bShouldOverrideExplosiveinfo = false;

	// 오버라이드 할 값
	UPROPERTY(EditDefaultsOnly)
	FExplosionInfo NewExplosiveInfo;

/* Damage */
public:
	// 발사체가 가하는 데미지 타입
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UTRDamageType> DamageType;

	// 발사체 데미지 수치
	UPROPERTY(EditDefaultsOnly)
	float EnemyDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float AllyDamage = 0.0f;

	// 부위별 추가 배율
	UPROPERTY(EditDefaultsOnly)
	float HeadshotMultiplier = 1.0f;

	// 거리 당 falloff
	UPROPERTY(EditDefaultsOnly)
	bool bHasDmgDistFallOff = true;

	UPROPERTY(EditDefaultsOnly)
	float DmgDistFallOffMult = 1.0f;

	// 거리에 따른 데미지 배율 조정
	UPROPERTY(EditDefaultsOnly)
	float DmgMultDistClose = 1.0f;

	UPROPERTY(EditDefaultsOnly)
	float DmgMultDistFar = 1.0f;

	// 캐릭터 적중 시 격발자 데미지 / 회복량
	UPROPERTY(EditDefaultsOnly)
	float SelfDmgOnCharacterHit = 0.0f;

	// 에어샷 시 데미지 배율
	UPROPERTY(EditDefaultsOnly)
	float DmgMultOnAirshot = 1.0f;

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

/* Misc */
protected:
	// 최초 생성 위치
	FVector InitialLocation;

/* Physics */
public:
	// 물리 충격량을 가하는지 여부
	UPROPERTY(EditDefaultsOnly)
	bool bShouldAffectForceOnCollision = true;

	// 충격량 연산 시 사용할 투사체 질량 (기타 연산에는 사용되지 않음)
	UPROPERTY(EditDefaultsOnly)
	float ImpactMass = 0.0f;	

	// 해당 횟수 이상 액터와 충돌 시 파괴된다
	// 1 미만의 값의 경우 몇 번 충돌하더라도 파괴되지 않는다
	// NOTE: 이 값을 1 미만으로 설정하기 전에 bPiercePawns 또는 bBounceOffIndefinitely의 사용을 고려할 것
	UPROPERTY(EditDefaultsOnly)
	int DestroyOnHitCount = 1;

	// 폰 관통 여부
	// true일 경우 폰과 충돌하더라도 Destruction hit count가 줄어들지 않는다
	// NOTE: 바운스와 같이 사용할 수 없다
	UPROPERTY(EditDefaultsOnly)
	bool bPiercePawns = false;

	// 물체에 한해 영구 바운스 허용 여부
	// true일 경우 벽이나 바닥에 튕기더라도 Destruction hit count가 줄어들지 않는다
	// NOTE: 애초에 투사체가 바운스 하지 않을 경우 아무 것도 수행하지 않는다
	UPROPERTY(EditDefaultsOnly)
	bool bBounceOffObjIndefinitely = false;

protected:
	// 현재 충돌 횟수
	int CurrHitCount = 0;

	// 오브젝트 충돌 VFX 재생을 해야하는지 여부
	bool bShouldPlayObjColVFX = true;

protected:
	// 충돌 시 처리 로직
	virtual void OnProjHitboxCollision(UPrimitiveComponent* HitComponent, class AGameCharacter* HitboxOwner, class UHitboxComponent* HitboxComp, FVector NormalImpulse, const FHitResult& Hit);
#pragma endregion

#pragma region /** VFX */
public:
	// FX 리소스
	UPROPERTY(EditDefaultsOnly)
	class UFxConfig* FxConfig = nullptr;

	// FxConfig 기반으로 런타임에서 바인딩해줄 수도 있고, 원하면 투사체에 따라 직접 바인딩 할 수 있다
	// 투사체 이펙트
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* ProjectileVFX = nullptr;

	// 궤적 이펙트
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* TrailVFX = nullptr;

	// 캐릭터 히트박스 충돌 시 이펙트
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* HitCharacterImpactVFX = nullptr;

	// 캐릭터가 아닌 지형지물 충돌 시 이펙트
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* HitObjectImpactVFX = nullptr;

protected:
	// Proj VFX 인스턴스
	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> Local_ProjInst = nullptr;

	// Trail VFX 인스턴스
	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> Local_TrailInst = nullptr;

protected:
	// 필요한 VFX들을 바인딩 및 스폰한다
	virtual void Local_InitializeVFX();

	// Destroy 시 제거가 필요한 VFX를 제거한다
	// Bullet trace의 경우 충돌 시 지워지는 게 정상이지만, Hit Impact의 경우 충돌 직후에도 남아 재생되어야 하므로
	// 관련된 모든 VFX를 지우는 게 아님에 유의해야 한다
	virtual void Local_CleanUpVFX();

	// Projectile VFX
	void Local_SpawnProjVFX();
	void Local_DestroyProjVFX();

	// Trail VFX
	void Local_SpawnTrailVFX();
	void Local_DestroyTrailVFX();
	
public:
	// Hit Impact VFX
	void Server_SpawnProjHitObjectVFX(FVector HitLocation, FRotator HitRotation);
	void Server_SpawnProjHitCharacterVFX(FVector HitLocation, FRotator HitRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnProjHitObjectVFX(FVector HitLocation, FRotator HitRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnProjHitCharacterVFX(FVector HitLocation, FRotator HitRotation);

protected:
	// true일 경우 VFX를 충돌 대상의 normal 벡터 방향에 맞게 생성한다
	// false일 경우 투사체의 이동 방향의 반대 방향에 맞게 생성한다
	UPROPERTY(EditDefaultsOnly)
	bool bUseHitNormalForVFX = false;
#pragma endregion
};
