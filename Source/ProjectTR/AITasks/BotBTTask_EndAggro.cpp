// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_EndAggro.h"
#include "BaseAIController.h"

UBotBTTask_EndAggro::UBotBTTask_EndAggro()
{
	NodeName = TEXT("End Aggro");
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBotBTTask_EndAggro::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (AIController)
	{
		AIController->OnAggroEnd();
	}
	return EBTNodeResult::Succeeded;
}

FString UBotBTTask_EndAggro::GetStaticDescription() const
{
	return FString();
}
