// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "TRMacros.h"
#include "TRDamageType.h"
#include "ReplicatedObject.h"
#include "StatusEffect.generated.h"

// 어떤 StatusEffect 인스턴스의 지속시간을 생성 이후 수정할 경우 어떻게 처리할지를 나타내는 정보
// e.g. 화염 디버프가 걸린 상태에서 화염 디버프에 한번 더 걸리면 둘 중 더 긴 시간으로 업데이트 해야 함
// 이때는 STHM_UseLargerVal 사용
UENUM(BlueprintType)
enum class EStatTimerHandleMethod : uint8
{
	STHM_ForceOverride UMETA(DisplayName = "Always override the duration value"),
	STHM_UseLargerVal UMETA(DisplayName = "Use larger value; Does not override if the new value is smaller"),
	STHM_UseSmallerVal UMETA(DisplayName = "Use smaller value; Does not override if the new value is larger"),
};

// 상태이상 하나가 가하는 데미지를 나타낸다
// FCharacterStatModifier과 다르게 델타를 더해 하나로 합치는 것이 불가능하다
USTRUCT(BlueprintType)
struct FStatEffectDamageInfo
{
	GENERATED_BODY()

public:
	// 데미지 연산 처리 시 가해지는 데미지 값
	// 이 값은 가변적이며, StatusEffect 부여 이후 변할 수 있음
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageVal = 0.0f;

	// 매 데미지 연산 처리 시의 데미지 값 증감
	// 최초 1회 연산에는 반영되지 않으므로 이를 고려해 값을 설정할 것
	// 덧셈을 먼저 처리, 곱셈을 이후에 처리함
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageDeltaPerCalc = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultPerCalc = 1.0f;

	// 데미지 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UTRDamageType> DamageType = nullptr;

	// 데미지가 한번 가해지는 데 걸리는 시간 (seconds)
	// 이 값이 음수일 경우 데미지 연산을 아예 처리하지 않음
	// 이 값이 0일 경우 데미지를 1회만 적용함
	// 이 값이 0 초과일 경우 해당하는 시간을 주기로 지속해서 반복해 데미지를 적용함
	// 이때 해당 값이 서버 틱보다 작을 경우(e.g. 0.00001)라도 최대 Rate는 서버 틱과 동일함 (60fps면 초당 60번)
	// NOTE: 데미지의 총 지속시간은 StatusEffect의 수명과 동일함
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageRateSec = -1.0f;
};

// 캐릭터에게 부여할 스테이터스 변화값 하나의 단위를 나타낸다
// 캐릭터의 최종 스테이터스는 베이스 스텟에 Stat modifier들의 합을 더한 값으로 계산된다 
USTRUCT(BlueprintType)
struct FCharacterStatModifier
{
	GENERATED_BODY()

/* 캐릭터 액터 */
public:
	// 저항
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaPhysicalRes = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaElementalRes = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaMagicalRes = 0.0f;

	// 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeltaMaxHealth = 0;

	// 이동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaMaxWalkSpeed = 0.0f; // 스프린트, 걷기 둘 다 적용되는 델타

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeltaJumpCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaJumpSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaRollSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaRollDelay = 0.0f;

/* 총기 */
// NOTE: GunPartComponent의 어트리뷰트와 동일한 기능을 하는 경우 네이밍도 동일하게 적용
public:
	// 추가 발사체
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeltaMissileSpawnedPerShot = 0;

	// 추가 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaDmgEnemyDirect = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaDmgAllyDirect = 0.0f;

	// 추가 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaSelfDmgOnCharacterHit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaSelfDmgOnCharacterMissedHitscan = 0.0f; // 히트스캔에만 적용

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaSelfDmgOnCharacterKilled = 0.0f;

	// 추가 조건부 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaDmgMultDistClose = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaDmgMultDistFar = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaDmgMultOnAirshot = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaDmgMultOnHead = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaDmgDistFallOffMult = 0.0f;

	// true일 경우에만 총기 설정값 오버라이드
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetHitscanPiercePawnsToTrue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetProjPiercePawnsToTrue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetExplodeOnHitToTrue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetHasDmgDistFallOffToFalse = false;

	// 폭발 추가값
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaGunExplosionRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaGunExplosionDamage = 0.0f;

	// 추가 연사 성능
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaFireInterval = 0.0f;

	// 추가 탄퍼짐
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaRecoilOffsetRange = 0.0f;

	// 추가 장탄
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeltaMaxAmmo = 0;

	// 추가 투사체 성능 조정치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaProjSpeed = 0.0f; // Initial, Max speed에 모두 적용

public:
	void AddToThis(const FCharacterStatModifier& Other)
	{
		DeltaPhysicalRes += Other.DeltaPhysicalRes;
		DeltaElementalRes += Other.DeltaElementalRes;
		DeltaMagicalRes += Other.DeltaMagicalRes;
		DeltaMaxHealth += Other.DeltaMaxHealth;

		DeltaMaxWalkSpeed += Other.DeltaMaxWalkSpeed;
		DeltaJumpCount += Other.DeltaJumpCount;
		DeltaJumpSpeed += Other.DeltaJumpSpeed;
		DeltaRollSpeed += Other.DeltaRollSpeed;
		DeltaRollDelay += Other.DeltaRollDelay;

		DeltaMissileSpawnedPerShot += Other.DeltaMissileSpawnedPerShot;
		DeltaDmgEnemyDirect += Other.DeltaDmgEnemyDirect;
		DeltaDmgAllyDirect += Other.DeltaDmgAllyDirect;
		DeltaSelfDmgOnCharacterHit += Other.DeltaSelfDmgOnCharacterHit;
		DeltaSelfDmgOnCharacterMissedHitscan += Other.DeltaSelfDmgOnCharacterMissedHitscan;
		DeltaSelfDmgOnCharacterKilled += Other.DeltaSelfDmgOnCharacterKilled;

		DeltaDmgMultDistClose += Other.DeltaDmgMultDistClose;
		DeltaDmgMultDistFar += Other.DeltaDmgMultDistFar;
		DeltaDmgMultOnAirshot += Other.DeltaDmgMultOnAirshot;
		DeltaDmgMultOnHead += Other.DeltaDmgMultOnHead;
		DeltaDmgDistFallOffMult += Other.DeltaDmgDistFallOffMult;

		bSetHitscanPiercePawnsToTrue |= Other.bSetHitscanPiercePawnsToTrue;
		bSetProjPiercePawnsToTrue |= Other.bSetProjPiercePawnsToTrue;
		bSetExplodeOnHitToTrue |= Other.bSetExplodeOnHitToTrue;
		bSetHasDmgDistFallOffToFalse |= Other.bSetHasDmgDistFallOffToFalse;

		DeltaGunExplosionRadius += Other.DeltaGunExplosionRadius;
		DeltaGunExplosionDamage += Other.DeltaGunExplosionDamage;
		DeltaFireInterval += Other.DeltaFireInterval;
		DeltaRecoilOffsetRange += Other.DeltaRecoilOffsetRange;
		DeltaMaxAmmo += Other.DeltaMaxAmmo;
		DeltaProjSpeed += Other.DeltaProjSpeed;
	}
};

// 스테이터스 이펙트 오브젝트 생성을 위해 사용되는 정보
USTRUCT(BlueprintType)
struct FStatEffectGenInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StatusEffectId = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StatusName = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StatusDesc = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStatEffectDamageInfo DamageInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterStatModifier StatModifier;

	// 0일 경우 타이머 설정 X
	// 아이템 장착을 통한 효과 부여처럼, 시간과 무관하게 반영구적으로 지속되는 효과에 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StatDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStatTimerHandleMethod TimerHandleMethod;

	// true인 경우 동일한 id의 인스턴스가 존재하더라도 새 인스턴스를 생성함 (즉 복수의 효과가 공존함)
	// false인 경우 기존 동일 id의 인스턴스를 찾아 TimerHandleMethod에 따라 duration만 수정함
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForceNewInstance = false;
};

/**
 * StatusEffect는 별도의 상속된 자식을 가지지 않는 것을 기준으로 디자인되었다
 * 게임플레이에 영향을 주는 Stateful한 정보는 전부 FCharacterStatModifier 내에 기록하는 것이 권장된다
 * 
 * 레플리케이트 되는 정보들을 제외한 모든 값들은 server-auth이다
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTTR_API UStatusEffect : public UReplicatedObject
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UStatusEffect, StatusEffectName);
		DOREPLIFETIME(UStatusEffect, StatusEffectDesc);
	}

public:
	// TODO: 현재는 레벨 트랜지션 사이에 StatusEffect는 유지되지 않음.
	// InvObject 내에 종속된 경우 유지되는 것처럼 보이지만, 이건 사실 인벤토리 export/import 과정을 통해 다시 복구되는 것에 불과함
	// 즉 그 외의 방법으로 획득한 상태이상 (일시적 중독, 화염 데미지 등)은 레벨 트랜지션 과정에서 삭제됨.
	void ChangeRootSetRecursive(bool bAddRoot, UObject* NewOuter);

	const FString& GetStatName() const { return StatusEffectName; }
	void SetStatName(const FString Name) { StatusEffectName = Name; }

	const FString& GetEffectDesc() const { return StatusEffectDesc; }
	void SetStatDesc(const FString Desc) { StatusEffectDesc = Desc; }

	const FStatEffectDamageInfo& GetEffectDamage() const { return EffectDamage; }
	void SetEffectDamage(const FStatEffectDamageInfo& NewEffectDamage) { EffectDamage = NewEffectDamage; }

	const FCharacterStatModifier& GetStatDelta() const { return StatDelta; }
	void SetStatDelta(const FCharacterStatModifier& NewStatDelta) { StatDelta = NewStatDelta; }

	const FString& GetStatusEffectId() const { return Server_StatusEffectId; }
	void SetStatusEffectId(const FString& Id);

	void Server_SetDuration(float Seconds, EStatTimerHandleMethod HandleMethod);
	void Server_AddDuration(float DeltaSeconds);

protected:
	// ID (중복될 수 있다)
	FString Server_StatusEffectId = "";

	// 실제 UI에 보여지는 값들
	UPROPERTY(Replicated)
	FString StatusEffectName = "";

	UPROPERTY(Replicated)
	FString StatusEffectDesc = "";

/* Gameplay */
public:
	// 캐릭터에 이 상태효과가 적용되었을 때 호출된다
	void Server_OnSelfAddedToParent(class AGameCharacter* Parent, class AGameCharacter* Applier);

	// 비정상적인 스테이터스가 없는지 확인한다
	void Server_ValidateSelf(float InitialStatDuration);

protected:
	// 스스로를 무효화한다 (상태 제거)
	void Server_InvalidateSelf();

	// NOTE: StatusEffect가 (플레이어 사망 등의 이유로) 타이머가 남았지만 도중에 Remove될 경우 이 콜백이 호출되지 않고 제거될 수 있다
	// 따라서 이 함수 내에 어떠한 경우에도 반드시 호출되어야 하는 로직이 들어가서는 안된다
	UFUNCTION()
	void Server_OnDurationEnd();

	// 등록된 DamageInfo에 따라 타깃에게 데미지를 1회 적용한다
	UFUNCTION()
	void Server_ApplyDamageOnce(AGameCharacter* Target, AGameCharacter* Applier);

public:
	// 시간 경과 시 자동으로 스테이터스 제거
	// duration이 없는 스테이터스 이펙트들의 경우에는 애초에 타이머를 작동시키지 않음
	FTimerHandle Server_DurationTimer;

	// 데미지 처리를 위해 사용되는 타이머
	FTimerHandle DamageTimer;

protected:
	// 이 이펙트가 부여된 대상에게 적용할 스테이터스 모디파이어
	// 이 값은 레플리케이트 할 필요가 없음, 로직에만 관여하기 때문
	UPROPERTY(EditDefaultsOnly)
	FCharacterStatModifier StatDelta;

	// 이 이펙트가 부여된 대상에게 적용할 데미지
	UPROPERTY(EditDefaultsOnly)
	FStatEffectDamageInfo EffectDamage;

	// 현재 이 상태이상이 부여된 캐릭터
	TWeakObjectPtr<class AGameCharacter> ParentChar = nullptr;

	// 이 상태이상을 부여한 캐릭터; ParentChar과 같을 수도, nullptr일 수도 있다
	TWeakObjectPtr<class AGameCharacter> ApplierChar = nullptr;
};
