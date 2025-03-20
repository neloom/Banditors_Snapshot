// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_PatrolLocUnreachable.h"
#include "TRMacros.h"
#include "BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_PatrolLocUnreachable::UBotBTTask_PatrolLocUnreachable()
{
	NodeName = TEXT("Patrol location is unreachable");
}

EBTNodeResult::Type UBotBTTask_PatrolLocUnreachable::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (IsValid(AIController))
	{
		AIController->ClearPatrolLocation();
	}

	// 태스크 종료
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

FString UBotBTTask_PatrolLocUnreachable::GetStaticDescription() const
{
	return FString("Resets desired patrol location");
}
