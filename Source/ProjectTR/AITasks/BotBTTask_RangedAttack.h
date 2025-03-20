// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BotBTTask_RangedAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UBotBTTask_RangedAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBotBTTask_RangedAttack();

private:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 틱
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// BP node description
	virtual FString GetStaticDescription() const override;

	// 공격 시도 여부
	bool ShouldTryAttack(class ABotCharacter* Bot, AActor* Target);

private:
	// 공격 시행을 개시하고 종료를 대기중인 경우 true로 설정한다
	bool bIsAttackPendingFinish = false;

	// 봇의 공격 시도 타깃이 공격 시도 거리 내인 경우에만 공격할 지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack trial dist checking", meta = (AllowPrivateAccess = true))
	bool bShouldCheckAtkTrialRange = false;

	// 조준선을 방해하는 것이 없는 경우에만 공격할 경우 true로 설정
	// 보스 같이 더 '지능적'인 몬스터들에 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check for LOS", meta = (AllowPrivateAccess = true))
	bool bShouldCheckLOS = false;
};
