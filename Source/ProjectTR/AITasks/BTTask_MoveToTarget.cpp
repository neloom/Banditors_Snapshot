// Copyright (C) 2024 by Haguk Kim


#include "BTTask_MoveToTarget.h"
#include "BotCharacter.h"
#include "AIController.h"

UBTTask_MoveToTarget::UBTTask_MoveToTarget()
{
	NodeName = TEXT("Custom Move To Node For Melee Attack");
	this->AcceptableRadius = INFINITY;
	bCreateNodeInstance = true; // 액터별로 프로퍼티 값이 달라질 수 있음
}

EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!bInitialized)
	{
		bInitialized = true;
		ABotCharacter* Bot = Cast<ABotCharacter>(OwnerComp.GetAIOwner()->GetPawn());
		if (Bot)
		{
			// 밀리 범위에만 들어와도 도착한 것으로 취급
			this->AcceptableRadius = Bot->Server_GetMeleeAtkRange();
		}
	}
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}