// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BotBTTask_Unexpected.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UBotBTTask_Unexpected : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBotBTTask_Unexpected();

private:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
