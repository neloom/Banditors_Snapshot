// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameCharacter.h"
#include "TRStructs.h"
#include "TREnums.h"
#include "BotCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ABotCharacter : public AGameCharacter
{
	GENERATED_BODY()
	
public:
	// 생성자
	ABotCharacter(const FObjectInitializer& ObjectInitializer);

	// 디액티베이트
	// 최적화를 위해 사용한다
	void Deactivate();

	// 액티베이트
	// 활성화를 하기 위한 용도로 사용된다
	void Activate();

	// 봇 머즐
	virtual TPair<FVector, FRotator> GetMuzzleInfo();

	// Getters
	bool GetBotActive() const { return bBotActive; }
	bool GetCanDespawn() const { return bCanDespawn; }

protected:
	// 디스폰 가능 여부
	UPROPERTY(EditAnywhere)
	bool bCanDespawn = true;

	// 봇에게 존재하는 가상의 머즐 위치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class URangedAttackComponent* RangeAtkComp = nullptr;

	// 봇의 활성화 여부
	bool bBotActive = false;

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginDestroy() override;

#pragma region /** Gameplay */
/* Health */
protected:
	virtual float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

/* Life */
protected:
	// 서버의 사망 시 게임 로직 처리
	virtual void Server_ProcessDeath() override;

	// 멀티캐스트 사망 시 게임 로직 처리
	virtual void Multicast_ProcessDeath() override;

/* Attack */
public:
	// 이 캐릭터의 AnimConfig에 대해 주어진 조건의 애니메이션 montage를 반환한다
	UAnimMontage* GetAnimMontage(EBotAnimType Type, uint8 Index);

/* Melee Attack */
public:
	// 이 봇이 근접공격이 가능한지 여부; 현재 가능한지 여부가 아닌, 행위 자체를 할 능력이 있는지를 반환
	bool CanEverPerformMeleeAtk() const { return bCanEverPerformMeleeAtk; }

	// 봇이 근접공격을 시도할 지 여부
	bool IsTargetInMeleeAtkTrialRange(const AActor* Target) const;

	// 근접 공격 처리
	UFUNCTION(Server, Reliable)
	void Server_BotMeleeAttack();

	/* Getters */
	bool Server_GetIsBotMeleeAttacking() { return bServer_IsBotMeleeAttacking; }

	UFUNCTION(BlueprintCallable)
	float Server_GetMeleeAtkRange() { return MeleeAttackRange; }

protected:
	// 밀리 공격 애니메이션 멀티캐스트
	// 몇번 애니메이션을 재생할지를 정수로 전달한다
	// 만약 잘못된 값이 주어졌을 경우 디폴트로 1번 밀리 애니메이션을 재생한다
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayBotMeleeAttackAnim(uint8 AnimIndex);

	// 애니메이션 종료 시 수행
	void EndBotMeleeAttackAnim(UAnimMontage* Montage, bool bInterrupted);

protected:
	// 공격 여부는 애니메이션의 재생 여부로 판정
	bool bServer_IsBotMeleeAttacking = false;

	// 이 봇의 밀리 공격을 위해 타깃에게 접근하는 거리
	// 밀리 사거리보다 작아야 한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MeleeAttackRange = 100.0f;

	// 이 봇의 밀리 공격 시도 범위
	// 실제 공격 범위보다 넓게 잡을 경우, 일종의 '위협성' 공격을 하는 효과를 줄 수 있다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MeleeAttackTrialRange = 700.0f;

	// 밀리 가능 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bCanEverPerformMeleeAtk = true;

/* Ranged Attack */
public:
	// 이 봇이 원거리 공격이 가능한지 여부; 현재 가능한지 여부가 아닌, 행위 자체를 할 능력이 있는지를 반환
	bool CanEverPerformRangedAtk() const { return bCanEverPerformRangedAtk; }

	// 원거리 공격 처리
	UFUNCTION(Server, Reliable)
	void Server_BotRangedAttack();

	// 머즐에서 타깃 사이에 투사체를 블로킹하는 장애물이 존재하는지 확인한다
	// 장애물이 발견될 경우 false를, 그 외에는 true를 반환한다
	bool IsAimingAtTarget(const AActor* Target);

	// 봇이 원거리공격을 시도할 지 여부
	bool IsTargetInRangedAtkRange(const AActor* Target) const;

	/* Getters */
	bool Server_GetIsBotRangedAttacking() { return bServer_IsBotRangedAttacking; }
	float Server_GetRangedAtkRange() { return RangedAttackRange; }
	float Server_GetRangedAtkRotPrecision() { return RangedAttackRotPrecision; }
	float Server_GetRangedAtkRotSpeed() { return RangedAttackRotSpeed; }

protected:
	// 원거리 공격 애니메이션 멀티캐스트
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayBotRangedAttackAnim(uint8 AnimIndex);

	// 애니메이션 종료 시 수행
	void EndBotRangedAttackAnim(UAnimMontage* Montage, bool bInterrupted);

protected:
	// 공격 여부는 애니메이션의 재생 여부로 판정
	// 공격 간 쿨다운과 별도의 개념
	bool bServer_IsBotRangedAttacking = false;

	// 이 봇의 원거리 공격 범위
	// 이는 실제 유효 사거리와 다를 수 있으며, 공격을 개시하는 거리를 의미한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RangedAttackRange = 1200.0f;

	// 이 봇의 원거리 조준 각도 허용 오차 (degree)
	// 이 값이 10이면 타깃으로부터 10도 회전이 어긋나 있어도 조준중인 것으로 취급한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RangedAttackRotPrecision = 1.0f;

	// 이 봇의 원거리 조준 속도 (degree)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RangedAttackRotSpeed = 6.0f;

	// 원거리 공격 가능 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bCanEverPerformRangedAtk = false;

/* Kill Rewards */
protected:
	// 처치 시 플레이어가 얻는 경험치
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rewards")
	int32 KillExpReward = 0;

	// 처치 시 드랍하는 아이템들과 각각의 드랍 확률
	// NOTE: 이는 소지 아이템을 드랍하는 것과 별개이다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rewards")
	TArray<FDropItem> DropRewards;

	// 아이템의 드랍 위치로부터의 랜덤 오프셋을 조정한다
	// 드랍 아이템의 수가 많을 경우 아이템간 물리 충돌을 막기 위해 값을 높여 설정할 수 있다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rewards")
	float RewardDropRandOffset = 0.0f;

	// 레벨 업 리워드로 드랍할 토큰 클래스
	// 기본값은 생성자에서 설정한다
	TSubclassOf<class ATRToken> DropTokenClass = nullptr;

public:
	// 주어진 캐릭터에게 킬 보상 경험치를 부여한다
	void Server_GiveKillExpRewardTo(class AFPSCharacter* Player);

	// 주어진 위치에 드랍 아이템들의 드랍 확률에 따라 랜덤으로 선택해 아이템을 스폰하고 드랍한다 (Deferred)
	void Server_SpawnAndDropRewardsAt(FVector Location);

protected:
	// 함수를 오버라이드해 마지막 공격자가 플레이어 캐릭터일 경우 킬 리워드 경험치를 보상한다
	virtual void Server_OnDeathHandleLastAttacker(AGameCharacter* Attacker) override;

protected:
	// 확률에 따라 랜덤으로 드랍할 아이템들을 선택한다
	const TArray<FDropItem> Server_SelectDropItems();
#pragma endregion
};
