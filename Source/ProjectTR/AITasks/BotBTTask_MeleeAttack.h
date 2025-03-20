// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BotBTTask_MeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UBotBTTask_MeleeAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBotBTTask_MeleeAttack();

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

	// 봇의 공격 시도 타깃이 공격 거리 내일 경우에만 공격할 지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack trial condition checking", meta = (AllowPrivateAccess = true))
	bool bShouldCheckAtkTrialRange = false;
};
