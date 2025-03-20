// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_OnLastLocVisit.h"
#include "BaseAIController.h"
#include "TRMacros.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_OnLastLocVisit::UBotBTTask_OnLastLocVisit()
{
	NodeName = TEXT("Target last known location visited or it is unreachable");
}

EBTNodeResult::Type UBotBTTask_OnLastLocVisit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (IsValid(AIController))
	{
		AIController->ClearTargetLastLocation();
	}
	
	// 태스크 종료
	return EBTNodeResult::Succeeded;
}

FString UBotBTTask_OnLastLocVisit::GetStaticDescription() const
{
	return FString("Resets target last known location and its validity");
}

