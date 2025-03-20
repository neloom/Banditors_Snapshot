// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MoveToTarget.generated.h"

/**
 * 밀리 공격을 위한 커스텀 이동 Task
 */
UCLASS()
class PROJECTTR_API UBTTask_MoveToTarget : public UBTTask_MoveTo
{
	GENERATED_BODY()
	
	UBTTask_MoveToTarget();

protected:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	// 초기화 여부
	bool bInitialized = false;
};
