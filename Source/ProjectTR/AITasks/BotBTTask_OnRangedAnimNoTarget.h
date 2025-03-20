// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BotBTTask_OnRangedAnimNoTarget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UBotBTTask_OnRangedAnimNoTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBotBTTask_OnRangedAnimNoTarget();

private:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 틱
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// 실행부 및 틱 공통 실행 로직
	EBTNodeResult::Type CoreTaskLogic(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

	// BP node description
	virtual FString GetStaticDescription() const override;
};
