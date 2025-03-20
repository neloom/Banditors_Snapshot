// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BotBTTask_TargetUpdate.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UBotBTTask_TargetUpdate : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBotBTTask_TargetUpdate();

private:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// BP node description
	virtual FString GetStaticDescription() const override;
};
