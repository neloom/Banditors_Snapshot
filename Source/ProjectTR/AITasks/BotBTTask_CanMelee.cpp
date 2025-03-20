// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_CanMelee.h"
#include "AIController.h"
#include "BotCharacter.h"
#include "TRMacros.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_CanMelee::UBotBTTask_CanMelee()
{
	NodeName = TEXT("Determine if the ai can ever perform melee attack");
}

EBTNodeResult::Type UBotBTTask_CanMelee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABotCharacter* Bot = Cast<ABotCharacter>(AIController->GetPawn());
	if (!Bot || !Bot->CanEverPerformMeleeAtk())
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Succeeded;
}

FString UBotBTTask_CanMelee::GetStaticDescription() const
{
	return FString("will fail if not possible");
}
