// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BotBTTask_MoveToRangedTarget.generated.h"

/**
 * 원거리 공격을 위한 커스텀 이동 Task
 */
UCLASS()
class PROJECTTR_API UBotBTTask_MoveToRangedTarget : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBotBTTask_MoveToRangedTarget();

protected:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
