// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_MoveToRangedTarget.h"
#include "BotCharacter.h"
#include "AIController.h"

UBotBTTask_MoveToRangedTarget::UBotBTTask_MoveToRangedTarget()
{
	NodeName = TEXT("Custom Move To Node For Ranged Attack");
	this->AcceptableRadius = INFINITY;
	bCreateNodeInstance = true; // 액터별로 프로퍼티 값이 달라질 수 있음
}

EBTNodeResult::Type UBotBTTask_MoveToRangedTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABotCharacter* Bot = Cast<ABotCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (Bot)
	{
		this->AcceptableRadius = Bot->Server_GetRangedAtkRange();
		this->ObservedBlackboardValueTolerance = this->AcceptableRadius * 0.95;
	}
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
