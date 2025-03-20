// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_CanRanged.h"
#include "AIController.h"
#include "BotCharacter.h"
#include "TRMacros.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_CanRanged::UBotBTTask_CanRanged()
{
	NodeName = TEXT("Determine if the ai can ever perform ranged attack");
}

EBTNodeResult::Type UBotBTTask_CanRanged::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABotCharacter* Bot = Cast<ABotCharacter>(AIController->GetPawn());
	if (!Bot || !Bot->CanEverPerformRangedAtk())
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Succeeded;
}

FString UBotBTTask_CanRanged::GetStaticDescription() const
{
	return FString("will fail if not possible");
}
