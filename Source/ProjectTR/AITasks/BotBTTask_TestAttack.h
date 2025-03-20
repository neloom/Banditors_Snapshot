// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BotBTTask_TestAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UBotBTTask_TestAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBotBTTask_TestAttack();

private:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 틱
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// BP node description
	virtual FString GetStaticDescription() const override;

	// 공격 시행을 개시하고 종료를 대기중인 경우 true로 설정한다
	bool bIsAttackPendingFinish = false;
};
