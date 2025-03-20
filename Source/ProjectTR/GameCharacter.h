// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RecoilAnimationComponent.h"
#include "BaseCharacterMovementComponent.h"
#include "TRStructs.h"
#include "TREnums.h"
#include "StatusEffect.h"
#include "Net/UnrealNetwork.h"
#include "GameCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameCharacterDeath, ACharacter*, DeadCharacter);

UENUM(BlueprintType)
enum class EHumanoidWeaponState : uint8
{
	NONE UMETA(DisplayName = "None"),
	LIGHT UMETA(DisplayName = "Light weapon"),
	HEAVY UMETA(DisplayName = "Heavy weapon"),
};

class UOuterHitboxComponent;
class UHitboxComponent;
class UCapsuleComponent;

UCLASS()
class PROJECTTR_API AGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// 생성자
	AGameCharacter(const FObjectInitializer& ObjectInitializer);

	// 재정의된 Character Movement Component 반환
	UFUNCTION(BlueprintCallable)
	FORCEINLINE class UBaseCharacterMovementComponent* GetTRCharacterMovementComponent() { return Cast<UBaseCharacterMovementComponent>(GetCharacterMovement()); }

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 현재 이 인스턴스의 정보를 추출해 반환한다
	// 인스턴스 데이터의 추출은 추출 대상의 내부 데이터를 변형시킬 가능성이 있기 때문에 
	// 레벨 트랜지션 처럼 더이상 기존 인스턴스를 사용하지 않는 경우에만 호출하는 것이 권장된다.
	virtual FGameCharacterInstanceData Server_GetInstanceData();

	// 인스턴스 정보를 기반으로 이 인스턴스를 복구하고 성공 여부를 반환한다
	virtual bool Server_RestoreFromInstanceData(const FGameCharacterInstanceData& InstData);

	// 주어진 시간 이후 이 캐릭터를 Destroy한다
	// 이 함수의 호출은 오직 Destroy가 가능한 상태의(전처리가 모두 끝난) 액터에 대해서만 호출되어야 한다
	void Server_RegisterDestructionAfter(float TimeLeft = 5.0f);

	// 이 캐릭터에 의해 다른 타깃에 데미지가 가해졌을 경우 호출됨
	virtual void Server_OnDamageInflictedToTarget(AGameCharacter* Target, FDamageEvent const& DamageEvent, int32 Damage, bool bIsKillshot, bool bIsCrit);

	// 이 캐릭터의 세부 히트박스 중 주어진 위치로부터 가장 가까운 대상을 반환한다
	class UHitboxComponent* GetNearestDetailedHitboxFrom(const FVector& WorldLocation);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 이 캐릭터에 연결된 컴포넌트들이 NavMesh의 생성에 영향을 줄 수 있는지 여부를 변경한다
	void SetCanAffectNavigation(bool Value);

	// 게임 캐릭터의 파괴에는 Destroy 대신 DestroySelf를 호출할 것이 강력하게 권장된다
	void DestroySelf();

	// 남은 모든 지연된 태스크를 처리한다
	// 일반적으로 파괴 직전 호출된다
	void ExecRemainingDeferredTask();

protected:
	// Destroy 예약을 위해 사용하는 타이머 핸들
	FTimerHandle DestructionTimerHandle;

	// 사망 이후 캐릭터의 Destroy 호출까지 기다리는 시간
	// NOTE: 이 값은 가급적 큰 값을 사용하는 게 권장된다
	// 이는 조금 더 래그돌 등장 시간을 늘리기 위해서이기도 하지만, Deferred 로직들이 수행될 시간을 충분히 주기 위함이 가장 크다
	float DestructionTimeDefault = 5.0f;

public:
	// 사망 시 이벤트 처리를 위해 외부 바인딩이 가능한 서버 전용 델리게이트
	// NOTE: 호출 시점은 사망 판정 직후 가장 먼저 처리됨
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameCharacterDeath Server_OnDeathDelegate;

#pragma region /** Networking */
protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AGameCharacter, AppliedStatEffects);
		DOREPLIFETIME(AGameCharacter, RealMaxHealthCached);
		DOREPLIFETIME(AGameCharacter, HealthPoint);
		DOREPLIFETIME(AGameCharacter, bHasDied);
		DOREPLIFETIME(AGameCharacter, RecoilComponent);
	}
/* RPCs */
// 플레이어 및 AI의 서버사이드 로직을 처리한다
public:
	// 호스트에서 RegisterFire를 호출하기 위한 최초 진입지점
	void Host_RegisterFire(bool bIsPrimary);

	// 서버에 요청을 보내기 전, 격발이 성공했다고 가정했을때 변하는 일부 값들을 변경한다
	// TriggerInterval과 같이 네트워크 딜레이보다 빠른 시간내에 처리해야 하는 값들 중
	// 실제 게임플레이 로직에 영향을 주지 않는 값들에 한정해 사용해야 한다
	// 이 함수는 로컬 클라이언트에서만 호출된다
	void Client_SimulateFire(bool bIsPrimary);

	// 공격(아이템 액션) RPC
	// 메인 액션인 경우 bIsPrimary를 true, 아니라면 false로 설정한다
	UFUNCTION(Server, Reliable)
	void Server_RegisterFireRPC(bool bIsPrimary);

	// 로컬 단위에서 현재 Fire 요청이 가능한지 확인한다
	// 클라이언트에서 FX를 재생하기 이전에 미리 확인하는 용도로 사용된다
	// 서버에서 호출될 경우 Register 된 요청을 Process해도 되는지 확인하기 위해 사용한다
	bool Host_CanPerformFire(bool bIsPrimary);

	// 공격(아이템 액션) 중단 RPC
	UFUNCTION(Server, Reliable)
	void Server_RegisterStopFireRPC(bool bIsPrimary);

	// Switch 시도 시 RPC
	UFUNCTION(Server, Reliable)
	void Server_RegisterSwitchToRPC(int32 SlotIdx);

	// 인터랙션 RPC
	// 현재 머즐 위치와 방향을 대상으로 상호작용 가능한 대상이 존재하는지 확인 후 처음 상호작용 가능한 대상에 대해 처리 가능한 행동을 처리한다
	UFUNCTION(Server, Reliable)
	void Server_InteractRPC();
#pragma endregion

#pragma region /** Gameplay */
public:
	// 손을 사용한 액션의 기준점을 반환한다
	virtual TPair<FVector, FRotator> GetHandPointInfo();

	// 조준의 기준점을 반환한다
	virtual TPair<FVector, FRotator> GetMuzzleInfo();

	// 캐릭터 '머리 위' 위치를 반환한다
	// 기본적으로 캡슐 컴포넌트의 맨 위 점을 반환한다
	virtual FVector GetTopOfHeadLocation();

	// 캐릭터가 공중에 있는지 여부를 반환한다
	bool IsInAir() const;

protected:
	// 그랩 중일 경우 대상이 캐릭터의 물리적 눈으로부터의 떨어져 있는 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	float GrabOffset = 100.0f;

	// 카메라 위치로부터의 실제 논리적 총구까지의 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	float MuzzleOffset = 0.0f;

protected:
	// 착지 시 로직 처리
	virtual void Landed(const FHitResult& Hit) override;	

/* Interaction */
protected:
	// 머즐 앞의 상호작용 가능한 액터들의 히트 결과를 반환한다
	virtual TArray<FHitResult> Reach();

	// 상호작용 가능한 액터들 중 처음으로 히트된 아이템 액터가 존재한다면 반환한다
	virtual class ABaseItem* ReachItem();

// 아이템
protected:
	// 아이템 픽업
	void Server_ProcessPickup(class ABaseItem* TargetItem);

	// 리치에 닿은 아이템에 대해 픽업 처리가 진행중일때, 해당 아이템에 픽업 전 해주어야 하는 처리를 진행한다
	virtual void ProcessBeforeItemPickup(class ABaseItem* ReachedItem);

// 던전 액터
protected:
	// 머즐 트리거 액터 처리
	void Server_ProcessMuzzleTrigger(class AMuzzleTriggeredActor* TargetActor);

// 아이템 드랍/스폰 지연
// 아이템 생성을 일괄에 처리하게 될 경우 생성 위치를 찾는 과정에서 큰 부하가 걸린다
// 때문에 이 작업을 여러 틱에 나누어 지연시키는 것으로 부하를 최소화한다
protected:
	// 드랍/스폰할 아이템들과 그 위치의 목록
	// NOTE: InvObject에 대한 참조는 한번에 한 곳에만 있어야 한다
	// 즉 인벤토리와 드랍 큐 양쪽 모두에 존재해서는 안된다
	TQueue<TPair<class UInvObject*, FVector>> Server_DropDeferredItemQueue;
	TQueue<TPair<class TSubclassOf<ABaseItem>, FVector>> Server_SpawnDeferredItemQueue;

	// 10fps 레이트로 처리한다
	const float DeferredTaskTickCycle = 0.1f;
	float CurrDeferredTickTime = 0.0f;

public:
	// 드랍/생성할 아이템 항목과 위치
	void Server_AddDeferredDropItem(class UInvObject* InvObj, FVector Location); // NOTE: 인벤토리에서의 제거는 별도로 처리해주어야 한다
	void Server_AddDeferredSpawnItem(TSubclassOf<class ABaseItem> ItemClass, FVector Location);

	// 큐에 쌓인 항목에 대한 드랍/생성 처리; MaxCount개 처리한다
	// 큐 전체에 대해 작업을 처리하기 위해서는 MaxCount에 큰 값을 전달하면 된다
	void Server_ExecuteDropForDeferredItem(int32 MaxCount);
	void Server_ExecuteSpawnForDeferredItem(int32 MaxCount);

protected:
	// 틱에서 실행할 함수
	void DeferredTaskTick(float DeltaTime);

/* Weapon switching */
public:
	// 무기 혹은 도구 슬롯 로직 처리
	void SwitchWeaponSlotTo(int32 SlotIdx);

/* Gun interaction */
public:
	// 모든 장착 및 보유한 총기의 탄약을 완전히 보충한다
	void Server_RefillAllAmmo();

/* Item attachment */
public:
	// 특정 소켓에 아이템을 부착하기 위해 사용
	void AttachItem(class ABaseItem* AttachItem);

	// 특정 아이템을 탈착하기 위해 사용; 현재 어느 소켓에 부착되어있는지와 무관하게 탈착한다
	void DetachItem(class ABaseItem* AttachedItem);

/* Health */
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	int32 PureMaxHealth = 100; // 실제 최대체력을 구하려면 GetStat_MaxHealth를 사용할 것

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UpdateHealth, Category = "Health")
	int32 HealthPoint = PureMaxHealth;

private:
	// 실제 최종 최대 최력값을 클라이언트에 전달하기 위한 캐시본;
	// 직접 액세스하지 말 것, GetStat_MaxHealth를 사용할 것
	UPROPERTY(ReplicatedUsing = OnRep_UpdateHealth)
	int32 RealMaxHealthCached = PureMaxHealth;

protected:
	// 체력 레플리케이션 콜백
	UFUNCTION()
	void OnRep_UpdateHealth();

	// 체력 값이 변했을 때 호출
	void OnHealthUpdate();

	// 체력이 0 이하가 되었을 때 호출
	void Server_OnHealthReachedZeroOrLower();

	// 함수 오버라이드
	// 음수 델타를 허용함
	virtual float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:
	// Getters
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE int32 GetStat_MaxHealth() const;
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE int32 GetStat_CurrentHealth() const { return HealthPoint; }

	// Setters
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetPureMaxHealthTo(const int32 Value);
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthPointTo(const int32 Value);

/* Life */
protected:
	// 이 캐릭터의 사망 여부
	// 사망은 오직 체력이 0 이하로 떨어지는 것에 의해서만 트리거 되어야 하며,
	// 직접 HasDied를 수정하는 행위는 가급적 피해야 함
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UpdateLife, Category = "Life")
	bool bHasDied = false;

protected:
	// 생명 레플리케이션 콜백
	UFUNCTION()
	void OnRep_UpdateLife();

	// 생명 상태가 변했을 때 호출
	void OnLifeUpdate();

	// 사망 판정 시 호출
	void Server_OnDeath();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeathRPC();

	// 서버의 사망 시 게임 로직 처리
	// 이 함수는 사망 이후 그 결과로써 호출되어야 하며, 이 함수가 사망 상태를 트리거하는 게 아님에 주의
	virtual void Server_ProcessDeath();

	// 멀티캐스트 사망 시 게임 로직 처리
	virtual void Multicast_ProcessDeath();

public:
	// Getters
	UFUNCTION(BlueprintPure, Category = "Life")
	FORCEINLINE bool GetHasDied() const { return bHasDied; }

	// Setters
	UFUNCTION(BlueprintCallable, Category = "Life")
	void SetHasDiedTo(const bool Value);

/* Damage */
public:
	// 유효한 값일 경우에만 포인터를 반환한다
	// 이 함수를 호출할 경우 Weakptr를 dereference한다
	AGameCharacter* Server_GetLastAttackedByAndDeref();

	// Weak ptr를 새로 생성해 할당한다
	void Server_SetLastAttackedBy(AGameCharacter* Character);

	// 실제 저항값 Getter
	// 저항값은 0에서 1 사이의 범위를 가진다
	float GetStat_PhysicalResistance() const { return FMath::Clamp(BasePhysicalResistance + StatusDeltaSumCached.DeltaPhysicalRes, 0.0f, 1.0f); }
	float GetStat_ElementalResistance() const { return FMath::Clamp(BaseElementalResistance + StatusDeltaSumCached.DeltaElementalRes, 0.0f, 1.0f); }
	float GetStat_MagicalResistance() const { return FMath::Clamp(BaseMagicalResistance + StatusDeltaSumCached.DeltaMagicalRes, 0.0f, 1.0f); }

protected:
	// 사망 시 마지막 공격자를 대상으로 무엇을 할지에 대한 로직
	virtual void Server_OnDeathHandleLastAttacker(AGameCharacter* Attacker);

	// 데미지 타입에 따른 데미지 증감 처리
	float Server_ModifyDamageByType(float OriginDamage, TSubclassOf<class UTRDamageType> DamageType) const;

protected:
	// 마지막으로 이 게임캐릭터를 공격한 대상을 기록한다
	TObjectPtr<AGameCharacter> Server_LastAttakedBy = nullptr;

private:
	// 데미지 타입별 기본 저항 (인스턴스 생성 이후 불변; 영구)
	// 인스턴스 생성 후 수정되지 않아야 한다
	UPROPERTY(EditDefaultsOnly)
	float BasePhysicalResistance = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float BaseElementalResistance = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float BaseMagicalResistance = 0.0f;

/* Melee Attack */
public:
	// 이 캐릭터가 성공적으로 공격한 대상에 대해 로직을 처리한다
	void ProcessMeleeAtk(AGameCharacter* Target, const FHitResult& MeleeHitResult);

protected:
	// 기본 맨손 근접 공격 데미지값
	float BaseMeleeDamage = 5.0f; // TODO TEMP

	// 기본 맨손 근접공격 랜덤 Additive 데미지값
	float AddMeleeDamage = 5.0f; // TODO TEMP

	// 근접 공격 데미지 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	TSubclassOf<class UTRDamageType> MeleeDamageType;


/* Dungeon */
public:
	// 던전 룸 옵저버 컴포넌트
	// 방에 진입하거나 탈출했을 경우를 감지해 필요 로직을 처리한다
	TObjectPtr<class UTRRoomObserverComponent> RoomObserverComp = nullptr;

public:
	// 룸에 진입하거나 탈출할 경우의 로직
	virtual void Server_OnRoomEnter(class ARoomLevel* RoomLevel);
	virtual void Server_OnRoomExit(class ARoomLevel* RoomLevel);

	// 마지막으로 방문한 룸 레벨 반환
	// 룸 옵저버 사용중이 아닐 경우 nullptr 반환
	class ARoomLevel* GetLastEnteredRoomLevel() const;

protected:
	// 모든 게임 캐릭터는 호스트 종류와 무관하게 기본적으로 룸 옵저버를 가지고는 있되, 사용하지 않는다
	// 이 값이 true일 경우 BeginPlay에서 옵저버를 Activate한다
	UPROPERTY(EditDefaultsOnly)
	bool bShouldActivateRoomObserver = false;

private:
	// 마지막으로 방문한 룸 레벨
	class ARoomLevel* LastEnteredRoomLevel = nullptr;
#pragma endregion

#pragma region /** Status */
public:
	// Getter
	UFUNCTION(BlueprintCallable)
	const TArray<class UStatusEffect*>& GetAppliedStatEffects() const { return AppliedStatEffects; }

protected:
	// 현재 캐릭터에게 적용되어있는 스테이터스 이펙트들
	UPROPERTY(Replicated)
	TArray<class UStatusEffect*> AppliedStatEffects;

public:
	// 조건에 따라 새 스텟 이펙트를 생성하고 추가한다
	// 새 스텟 이펙트 인스턴스가 생성된 경우 해당 인스턴스 포인터를 반환한다
	// 오류 발생, 혹은 정상처리 되었으나 새 인스턴스가 생기지는 않은 경우 nullptr를 반환한다
	// EffectApplier는 해당 상태이상을 부여한 부여자가 존재할 경우 전달되며, 
	// 이 값은 없을 수도(e.g. 지형지물) 혹은 자기 자신일 수도(e.g. 스스로 포션 마심) 있다 
	class UStatusEffect* Server_GenerateAndAddStatEffect(const FStatEffectGenInfo& Data, AGameCharacter* EffectApplier);

	// 부여된 스텟 이펙트를 제거한다
	void Server_RemoveStatEffect(class UStatusEffect* StatEffect);

protected:
	// 주어진 아이디를 가지는 스테이터스 이펙트가 존재하는지 찾고, 존재한다면 반환한다
	class UStatusEffect* Server_FindStatEffectOfId(const FString& Id);

	// 모든 스텟 이펙트를 제거한다
	// 일반적으로 이는 사망 시에만 호출되며, 그 밖의 상황에 강제로 호출하는 것은 권장되지 않는다
	// 이는 아이템과 스테이터스가 연결되어 있기 때문에, 자칫 스테이터스가 꼬이는 일이 발생할 수 있기 때문이다
	// (사망 시에는 인벤토리 전체 드랍이 보장되기 때문에 문제 없음)
	void Server_ClearAllStatEffect();

	// 이 캐릭터에게 부여된 스텟 이펙트들의 종류나 개수가 변화했을 경우 호출된다
	// 캐싱된 값들을 전부 재계산하고, 필요 시 Refresh해야 하는 로직을 수행한다
	void Server_OnStatEffectChanged();

	// 모든 캐시된 스테이터스 델타값들을 0으로 초기화한다
	void Server_ResetDeltaStatusSumCached();

	// 하나의 스테이터스 델타값을 이 캐릭터의 캐시값에 더한다
	void Server_AddStatusDeltaToSum(const FCharacterStatModifier& StatModifier);

/* 스테이터스 변화량 */
protected:
	// 현재 적용된 모든 버프/디버프들의 변화량들의 총합을 캐싱한 값으로, 
	// 캐릭터 스테이터스 변화 시 반드시 새로 갱신해주어야 한다
	// 불리안 값의 경우 or 연산합의 결과를 나타낸다
	FCharacterStatModifier StatusDeltaSumCached;

public:
	const FCharacterStatModifier& GetStatusDeltaSum() const { return StatusDeltaSumCached; }
#pragma endregion

#pragma region /** Animation */
public:
	// 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<class UAnimConfig> AnimConfig = nullptr;

/* Roll */
public:
	// Roll (NOT using client prediction)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Roll(float Forward, float Right);

/* Item */
public:
	// 현재 꺼낼 아이템에 맞는 적당한 애니메이션 몽타주를 가져온다
	UAnimMontage* GetDeployAnimMontage(EHumanoidWeaponState WeaponState);

	// 현재 WeaponEquipState에 맞는 애니메이션을 재생한다
	void PlayDeployAnimMontage();

	// 아이템 꺼내기 Montage 완료 시 처리
	void EndDeployAnimMontage(UAnimMontage* Montage, bool bInterrupted);

/* General */
public:
	// 이 캐릭터의 스켈레탈 메쉬에 애님 클래스를 링크시킨다
	UFUNCTION(Server, Reliable)
	void Server_SwitchAnimClassLayerRPC(EAnimClassType ClassType);

protected:
	// 공통 로직
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SwitchAnimClassLayerRPC(EAnimClassType ClassType);

	// 특정 AnimClassType에 대응되는 AnimClass를 반환한다
	virtual TSubclassOf<UAnimInstance> GetAnimClassOfType(EAnimClassType ClassType);
#pragma endregion

#pragma region /** Ragdoll */
protected:
	// 스켈레톤 메시에 피직스 에셋이 부착되어 있을 경우 래그돌 피직스를 가동한다
	void Server_Ragdollfy();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RagdollfyRPC();

	// 스켈레톤 메시를 제외한 모든 컴포넌트 작동 중지
	void DisableAllComponentsExceptSkeletalMesh();
#pragma endregion

#pragma region /** Collision */
public:
	// 외곽 히트박스
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UOuterHitboxComponent> OuterHitbox = nullptr;

	// 머리
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> HeadColComponent = nullptr;
	FTransform DeltaRelativeHead;

	// 상체
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> TorsoColComponent = nullptr;
	FTransform DeltaRelativeTorso;

	// 골반
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> PelvisColComponent = nullptr;
	FTransform DeltaRelativePelvis;

	// 오른팔 상부
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> RArmUpperColComponent = nullptr;
	FTransform DeltaRelativeRArmUpper;

	// 오른팔 하부
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> RArmLowerColComponent = nullptr;
	FTransform DeltaRelativeRArmLower;

	// 오른손
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> RHandColComponent = nullptr;
	FTransform DeltaRelativeRHand;

	// 왼팔 상부
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> LArmUpperColComponent = nullptr;
	FTransform DeltaRelativeLArmUpper;

	// 왼팔 하부
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> LArmLowerColComponent = nullptr;
	FTransform DeltaRelativeLArmLower;

	// 왼손
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> LHandColComponent = nullptr;
	FTransform DeltaRelativeLHand;

	// 오른다리 상부
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> RLegUpperColComponent = nullptr;
	FTransform DeltaRelativeRLegUpper;

	// 오른다리 하부
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> RLegLowerColComponent = nullptr;
	FTransform DeltaRelativeRLegLower;

	// 오른발
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> RFootColComponent = nullptr;
	FTransform DeltaRelativeRFoot;

	// 왼다리 상부
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> LLegUpperColComponent = nullptr;
	FTransform DeltaRelativeLLegUpper;

	// 왼다리 하부
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> LLegLowerColComponent = nullptr;
	FTransform DeltaRelativeLLegLower;

	// 왼발
	UPROPERTY(EditDefaultsOnly, Category = "Detailed Collision")
	TObjectPtr<UHitboxComponent> LFootColComponent = nullptr;
	FTransform DeltaRelativeLFoot;

	// 세부 히트박스 모음
	TArray<TObjectPtr<UHitboxComponent>> DetailColComponents;

/* Optimization */
public:
	// 주어진 시간동안 세부 히트박스를 활성화한다
	// 외곽 히트박스에 유의미한 콜리전이 발생 시, 보다 세부적인 로직을 처리할 준비를 하기 위해 사용할 수 있다
	// NOTE: 사용할 경우 동일한 콜리전 연산을 재실행해 주어야 활성화된 컴포넌트들이 타깃팅될 수 있다
	void ActivateDetailedHitboxFor(float Duration);
	void ActivateDetailHitboxForTick();

private:
	void ActivateDetailedHitbox();

protected:
	// 모든 히트박스의 최초 초기화
	// NOTE: 생성자 호출 전용
	void InitHitbox();

private:
	// 에디터에서 설정해둔 히트박스들의 상대 트랜스폼 값들을 캐싱한다
	// 이는 히트박스 위치를 계산하기 위해 사용된다
	void CacheHitboxDeltaRelativeTransform();

	// 세부 히트박스를 비활성화한다
	// 최초 1회 실행되는 것과 Activation 후 타이머에 의해 콜백 호출되는 것을 제외하면, 
	// 이 함수는 일반적으로 직접 호출해서는 안된다
	UFUNCTION()
	void DeactivateDetailedHitbox();

	// 소켓에 히트박스 부착
	// NOTE: 생성자 호출 전용
	void SetupHitboxToBone();

	// 런타임에 본에 히트박스 부착했을 때의 위치 및 회전 상태를 반영
	void SnapHitboxToBone();

	// 히트박스들을 본에 SnapToTarget 설정으로 어태치할 경우의 결과 트랜스폼을 호출 즉시 적용한다
	// 본 위치 기준 상대 트랜스폼 값을 함께 적용한다
	FORCEINLINE void SetHitboxTransformToBoneSingle(UHitboxComponent* Hitbox, USkeletalMeshComponent* TargetMesh, FName TargetBoneName, FTransform RelativeTransform);
	void SetHitboxTransformToBoneImmediate();

	// 히트박스 부위별 데미지 배율 초기화
	// 이 함수는 오직 GameCharacter의 생성자에서만 호출해야 함
	UFUNCTION(BlueprintCallable)
	void InitHitboxDmgMultipliers();

	// 한 부위에 대해 데미지 배율 값을 설정함
	void SetHitboxDmgMultiplier(UHitboxComponent* Hitbox, float Multiplier);

private:
	// 디테일 히트박스가 현재 활성화 되어있는지 여부
	bool bIsDetailedHitboxActivated = false;

	// 히트박스 활성화 주기 관리 타이머
	FTimerHandle DetailedHitboxDeactivateTimer;

/* Damage multipliers */
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float HeadDmgMult = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float TorsoDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float PelvisDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float RArmUpperDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float RArmLowerDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float RHandDmgMult = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float LArmUpperDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float LArmLowerDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float LHandDmgMult = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float RLegUpperDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float RLegLowerDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float RFootDmgMult = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float LLegUpperDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float LLegLowerDmgMult = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detailed Collision")
	float LFootDmgMult = 0.5f;

/* 커스텀 스켈레톤 사용 시 소켓 이름을 맵핑 */
// NOTE: 이 값을 블루프린트에서 편집할 경우 CDO 생성 단계에 적용되지 않아 에디터 상에서는 표기되지 않을 수 있음
// 고로 권장되는 방법은 하위 클래스를 생성해 해당 클래스의 생성자에서 직접 수정하는 방식임
protected:
	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString HeadSockName = "head";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString TorsoSockName = "spine_01";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString PelvisSockName = "pelvis";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString RArmUpperSockName = "upperarm_r";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString RArmLowerSockName = "lowerarm_r";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString RHandSockName = "hand_r";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString LArmUpperSockName = "upperarm_l";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString LArmLowerSockName = "lowerarm_l";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString LHandSockName = "hand_l";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString RLegUpperSockName = "thigh_r";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString RLegLowerSockName = "calf_r";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString RFootSockName = "foot_r";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString LLegUpperSockName = "thigh_l";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString LLegLowerSockName = "calf_l";

	UPROPERTY(BlueprintReadOnly, Category = "SocketName")
	FString LFootSockName = "foot_l";
#pragma endregion

#pragma region /** Inventory */
public:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<class UInventoryComponent> InvComponent = nullptr;

protected:
	// 인벤토리의 모든 아이템을 드랍한다
	void Server_DropAllItemFromInv(float RandomOffsetRange, bool bDeferred = true);

protected:
	// 드랍 오프셋
	float OnDeathDropOffset = 10.0f;
#pragma endregion

#pragma region /** Equipments */
public:
	UPROPERTY(BlueprintReadOnly, Category = "Equipments")
	TObjectPtr<class UEquipSystem> EquipSystem = nullptr;

	// 이 캐릭터의 특정 기능을 하는 부위에 대응되는 소켓을 반환한다
	// 다양한 형태의 캐릭터에 대해 공통된 기능의 부위가 공통된 역할을 하게 만들기 위해 사용한다
	// 가령 ECP_PrimaryWield는 인간의 손에 대응되는, 행동을 할때 주로 사용하는 기관의 위치를 반환한다
	virtual FName GetSockNameOfPart(ECharacterParts Part);

	// 이 캐릭터에게 해당하는 이름의 소켓이 존재한다면 그대로 반환하고, 없다면 기본값으로 사용할 소켓의 이름을 반환한다
	virtual FName TryGetSocketName(FName SocketName);

protected:
	// Deploy중인 아이템을 Retrieve하고, 모든 장착된 아이템을 드랍한다
	void Server_DropAllEquippedItems(float RandomOffset, bool bDeferred = true);

public:
	// 현재 장착하고 있는 무기의 타입에 따라 값이 결정된다
	// 클라이언트 사이드 값이다
	UPROPERTY(BlueprintReadOnly)
	EHumanoidWeaponState WeaponEquipState = EHumanoidWeaponState::NONE;
#pragma endregion

#pragma region /** FX / Recoil */
public:
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	TObjectPtr<class URecoilAnimationComponent> RecoilComponent = nullptr;

	// 아이템 사용 시 FX 및 반동 처리 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayItemFxRPC(bool bIsPrimary, APlayerController* InvokeHost);

	// 반동 애니메이션 중지 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopItemFxRPC(bool bIsPrimary, APlayerController* InvokeHost);

	// 실제 FX 처리 로직
	// FX를 재생한다
	void Local_PlayItemFx(bool bIsPrimary);

	// FX를 중지한다
	void Local_StopItemFx(bool bIsPrimary);

	// Recoil component를 초기화한다
	void Local_InitRecoil(const FRecoilAnimData Data, const float Rate, const int Bursts);
#pragma endregion

#pragma region /** UI */
public:
	// NOTE: 아래 함수들은 가급적 TRHUDWidget에 의해서만 호출되어야 한다
	void BindHUD(class UTRHUDWidget* HUD);
	void UnbindHUD();
	TWeakObjectPtr<class UTRHUDWidget> Local_GetBoundHUDWidget() { return Local_BoundHUDWidget; }

private:
	// 현재 이 캐릭터에 바인딩된 HUD
	// 어떠한 경우에도 HUD는 단 하나 존재해야 하고, 하나의 캐릭터에게만 바인딩되어야 한다
	// 일반적인 경우 HUD는 FPSCharacter에게만 바인딩된다
	// 그러나 구조적으로 어떤 캐릭터던 HUD에 정보를 표현할 수 있도록 제작함
	TWeakObjectPtr<class UTRHUDWidget> Local_BoundHUDWidget = nullptr;
#pragma endregion

#pragma region /** Debug */
#pragma endregion
};
