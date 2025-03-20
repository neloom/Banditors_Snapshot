// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_OnRangedAnimNoTarget.h"
#include "AIController.h"
#include "BotCharacter.h"
#include "TRMacros.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_OnRangedAnimNoTarget::UBotBTTask_OnRangedAnimNoTarget()
{
	NodeName = TEXT("Task called when ranged anim is playing without target");
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBotBTTask_OnRangedAnimNoTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	return CoreTaskLogic(OwnerComp, NodeMemory);
}

void UBotBTTask_OnRangedAnimNoTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	CoreTaskLogic(OwnerComp, NodeMemory);
}

EBTNodeResult::Type UBotBTTask_OnRangedAnimNoTarget::CoreTaskLogic(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABotCharacter* Bot = Cast<ABotCharacter>(AIController->GetPawn());
	if (!Bot)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// 로직 처리
	if (!Bot->Server_GetIsBotRangedAttacking())
	{
		AIController->GetBlackboardComponent()->SetValueAsBool(TEXT(AI_IS_RANGED_ANIM_PLAYING), false); // 원거리 공격 애니메이션 재생 종료, 재생 종료 시점에 타깃이 유효하지 않을 경우
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}

	// 태스크 콜백 호출전까지 반복
	return EBTNodeResult::InProgress;
}

FString UBotBTTask_OnRangedAnimNoTarget::GetStaticDescription() const
{
	return FString("The task will detect if ranged animation is over, and if so change blackboard key and return success.");
}
