// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRMacros.h"
#include "Net/UnrealNetwork.h"
#include "ExpComponent.generated.h"

struct ExpConst
{
	static constexpr int EXP_MIN_REQ = 10;
	static constexpr float EXP_REQ_POWER = 1.2;
	static constexpr float EXP_REQ_MULTIPLIER = 1.5;
	static constexpr int EXP_REQ_ADDITIVE = 10;
	static constexpr int EXP_MARKPOINT_LVL = 20;
	static constexpr int EXP_LVL_MIN = 1;
	static constexpr int EXP_LVL_MAX = 999;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTTR_API UExpComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExpComponent();

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UExpComponent, Experience);
		DOREPLIFETIME(UExpComponent, LevelExperience);
		DOREPLIFETIME(UExpComponent, Level);
		DOREPLIFETIME(UExpComponent, Shard);
	}

protected:
	virtual void BeginPlay() override;

/* Getters */
public:
	// 주어진 레벨에 대해 다음 레벨까지의 총 필요 경험치
	UFUNCTION(BlueprintCallable)
	const int GetTotalExpReqToLvlup(int32 CurrLevel) const;

	// 현재 이 컴포넌트가 다음 레벨로 레벨업하기 위해 남은 필요 획득 경험치
	UFUNCTION(BlueprintCallable)
	const int GetCurrExpReqToLvlup() const;

	// 현재 레벨
	UFUNCTION(BlueprintCallable)
	const int GetLevel() const { return Level; }

	// 현재 누적 경험치
	UFUNCTION(BlueprintCallable)
	const int GetCurrTotalExp() const { return Experience; }

	// 현재 레벨 경험치
	UFUNCTION(BlueprintCallable)
	const int GetCurrLevelExp() const { return LevelExperience; }

	// 현재 샤드
	UFUNCTION(BlueprintCallable)
	const int GetCurrShard() const { return Shard; }

/* Setters */
// NOTE: 통상 게임플레이 로직을 위해서는 Setter 대신 Logic 함수를 사용할 것
public:
	void SetLevel(int Value);
	void SetCurrTotalExp(int Value);
	void SetCurrLevelExp(int Value);
	void SetCurrShard(int Value);

/* Logic */
public:
	// 경험치 및 샤드를 획득한다
	// 요구량 이상일 경우 재귀적으로 레벨을 상승시킨다
	void GainExp(int32 Exp, float Multiplier = 1.0f);

	// 샤드를 획득한다
	// 경험치는 얻지 않는다
	// 통상적인 경험치 획득을 통한 샤드 획득의 경우 이 함수 대신 GainExp를 사용해야 한다
	void GainShard(int32 Value);

	// 샤드를 소비한다
	// 경험치는 잃지 않는다
	void SpendShard(int32 Value);

	// 레벨 경험치를 잃는다
	void LoseLevelExp();

	// 레벨을 포함한 모든 경험치를 초기값으로 리셋한다
	void ResetAllExp();

	// 레벨을 1 상승시킨다
	void LevelUp();

protected:
	UFUNCTION()
	void OnRep_Experience();
	void Local_OnExperienceUpdated();

	UFUNCTION()
	void OnRep_LevelExperience();
	void Local_OnLvlExperienceUpdated();

	UFUNCTION()
	void OnRep_Level();
	void Local_OnLevelUpdated();

	UFUNCTION()
	void OnRep_Shard();
	void Local_OnShardUpdated();

/* Networking */
protected:
	// NOTE: 아래 값들에 대한 write 연산은 불가피한 경우가 아닌 이상 지정된 Setter를 사용할 것

	// 누적 총 획득 경험치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Experience)
	int32 Experience = 0;

	// 현재 레벨에 대한 총 획득 경험치 (레벨 별 경험치)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_LevelExperience)
	int32 LevelExperience = 0;

	// 레벨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Level)
	int32 Level = ExpConst::EXP_LVL_MIN;

	// 현재 소비 가능 경험치(영혼파편)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Shard)
	int32 Shard = 0;
};
