// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "FxConfig.h"
#include "TRMacros.h"
#include "Chaos/ChaosEngineInterface.h"
#include "TRDamageType.h"
#include "DamageTypeNeutral.h"
#include "TRExplosion.generated.h"

UENUM(BlueprintType)
enum class ERadialImpulseFalloffWrapper : uint8
{
	RIFW_Constant = ERadialImpulseFalloff::RIF_Constant,
	RIFW_Linear = ERadialImpulseFalloff::RIF_Linear,
	RIFW_MAX = ERadialImpulseFalloff::RIF_MAX
};

USTRUCT(BlueprintType)
struct FExplosionInfo
{
	GENERATED_BODY()

public:
	// 생성 직후 폭발 여부
	UPROPERTY(EditDefaultsOnly)
	bool bExplodeOnBeginPlay = true;

	// 폭발 후 파괴 여부
	UPROPERTY(EditDefaultsOnly)
	bool bDestroyAfterExplosion = true;

	// 최대 폭발 거리
	UPROPERTY(EditDefaultsOnly)
	float ExplosionRadius = 100.0f;

	// 폭발 시 데미지 적용 여부
	UPROPERTY(EditDefaultsOnly)
	bool bApplyDamageOnExplosion = false;

	// 기본 폭발 데미지
	UPROPERTY(EditDefaultsOnly)
	float BaseDamage = 0.0f;

	// 폭발 피해 증감폭
	// 폭발의 가장 외곽에 위치했을 때 받는 데미지의 배율을 의미한다
	// 폭발의 중심점에서 배수는 1이 되며, 가장자리까지 선형적으로 값이 변화한다
	UPROPERTY(EditDefaultsOnly)
	float MinExplosionMultiplier = 0.25f;

	// 폭발 생성자에게의 데미지 배율 (자가피해)
	// 이때 생성자가 플레이어일 경우 팀원에게도 동일한 값을 사용한다
	UPROPERTY(EditDefaultsOnly)
	float DmgMultOnExplInstigator = 0.0f;

	// 폭발 시 물리 충격 적용 여부
	// TODO: 액터 타입별로 충격량 적용을 다르게 처리; 로켓점프 등
	UPROPERTY(EditDefaultsOnly)
	bool bApplyImpactOnExplosion = false;

	// 중심점에서의 물리 충격량
	UPROPERTY(EditDefaultsOnly)
	float BaseImpactStrength = 0.0f;

	// 폭발 데미지 타입 (기본값 Neutral)
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UTRDamageType> ExplosionDamageType = UDamageTypeNeutral::StaticClass();

	// 충격량 증감폭
	// NOTE: ERadialImpulseFalloff를 직접 프로퍼티로 노출시킬 수 없음
	UPROPERTY(EditDefaultsOnly)
	ERadialImpulseFalloffWrapper ImpactFalloffType = ERadialImpulseFalloffWrapper::RIFW_Linear;

	// 폭발 효과를 막을 수 있는 액터 콜리전 타입
	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<ECollisionChannel>> ExplosionBlockedByType = { ECC_WorldStatic, ECC_WorldDynamic };

	// 폭발 적용 대상의 콜리전 타입
	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<ECollisionChannel>> ExplosionTargetType = { ECC_WorldDynamic, ECC_PlayerPawn, ECC_BotPawn, ECC_PhysicsBody, ECC_Item };

	// VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ENiagaraReference ExplosionVFXEnum = ENiagaraReference::ENR_NULL;

	// 폭발 VFX는 폭발 액터의 크기 및 폭발 반경에 비례한다; 
	// 이때 VFX의 크기가 실제 폭발 크기와 유사하도록 조정하기 위해 곱해지는 상수값임
	UPROPERTY(EditDefaultsOnly)
	float VFXRadiusConstant = 1.0f;
};


UCLASS()
class PROJECTTR_API ATRExplosion : public AActor
{
	GENERATED_BODY()
	
public:
#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		/* 클라이언트 VFX */
		DOREPLIFETIME_CONDITION(ATRExplosion, ExplosionVFX, COND_InitialOnly);
		DOREPLIFETIME_CONDITION(ATRExplosion, ExplosionVFXScale, COND_InitialOnly);
	}
#pragma endregion

public:
	ATRExplosion();
	virtual void Tick(float DeltaTime) override;

	// 폭발 정보를 새로 덮어쓴다
	void SetExplosionInfo(FExplosionInfo NewInfo) { ExplosionInfo = NewInfo; }

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 폭발 처리
	virtual void Server_Explode();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Explode();

	// 대상에게 가해지는 데미지 계산
	// NOTE: 여기서는 폭발 자체의 특성에 대한 데미지 연산만 처리한다
	// 예시로 거리 별 Damage Falloff에 대한 것은 처리될 수 있으나, 폭발 저항에 따른 데미지 경감은 어떤 특정 액터에 종속적이므로 여기서는 처리하지 않는다
	// 반면 폭발 유발자(instigator)에 대한 데미지 증감은 폭발 자체에 저장된 정보를 기반으로 처리하므로 여기서 처리한다
	float GetDamageOnTarget(class AGameCharacter* Target);

	// 타깃이 유효한 액터에 가려져 폭발로부터 영향을 받지 않는지 여부 확인
	bool IsBlocked(FVector StartLocation, FVector TargetLocation);

	// 폭발 VFX 재생
	void Local_PlayExplosionVFX();

protected:
	// NOTE: 루트 컴포넌트가 SceneComponent가 아닐 경우 유효한 트랜스폼을 가지지 못하는 문제가 발생함
	// 때문에 불가피하게 루트 컴포넌트를 하나 가져야 함
	TObjectPtr<USceneComponent> ExplRootComp = nullptr;

	UPROPERTY(EditAnywhere)
	FExplosionInfo ExplosionInfo;

	// 블록 체킹 시 콜리전 노말 방향으로 얼마만큼 이동한 곳을 원점으로 설정할지 결정
	UPROPERTY(EditAnywhere)
	float BlockCheckDeltaSize = 24.0f;

	UPROPERTY(EditDefaultsOnly)
	class UFxConfig* FxConfig = nullptr;
	
	// 폭발에 사용할 VFX; 런타임에 ExplosionInfo 기반으로 바인딩된다
	UPROPERTY(Replicated)
	class UNiagaraSystem* ExplosionVFX = nullptr;
	
	UPROPERTY(Replicated)
	FVector ExplosionVFXScale = FVector::OneVector;

public:
	// 이 폭발이 충돌에 의해 발생했을 경우 충돌 발생 시점의 노말값을 사용해 블로킹을 체크한다
	// 이는 액터 스폰 지점을 기준으로 체크할 경우 벽이나 바닥에 블로킹 되는 문제가 발생하기 때문임
	FVector CollisionNormal = { 0.f, 0.f, 0.f };
};
