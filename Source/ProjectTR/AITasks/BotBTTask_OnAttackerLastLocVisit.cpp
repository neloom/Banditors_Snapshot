// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_OnAttackerLastLocVisit.h"
#include "BaseAIController.h"
#include "TRMacros.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_OnAttackerLastLocVisit::UBotBTTask_OnAttackerLastLocVisit()
{
	NodeName = TEXT("Attacker last known location visited or it is unreachable");
}

EBTNodeResult::Type UBotBTTask_OnAttackerLastLocVisit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (IsValid(AIController))
	{
		AIController->ClearAttackerLastLocation();
	}

	// 태스크 종료
	return EBTNodeResult::Succeeded;
}

FString UBotBTTask_OnAttackerLastLocVisit::GetStaticDescription() const
{
	return FString("Resets attacker last known location and reference");
}

