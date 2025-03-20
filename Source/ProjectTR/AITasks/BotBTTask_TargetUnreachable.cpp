// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_TargetUnreachable.h"
#include "TRMacros.h"
#include "BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_TargetUnreachable::UBotBTTask_TargetUnreachable()
{
	NodeName = TEXT("Target is unreachable");
}

EBTNodeResult::Type UBotBTTask_TargetUnreachable::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (IsValid(AIController))
	{
		AIController->ClearTarget();
	}

	// 태스크 종료
	return EBTNodeResult::Failed;
}

FString UBotBTTask_TargetUnreachable::GetStaticDescription() const
{
	return FString("LEGACY: use TargetUpdate instead");
}
