// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_OnMeleeAnimNoTarget.h"
#include "AIController.h"
#include "BotCharacter.h"
#include "TRMacros.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_OnMeleeAnimNoTarget::UBotBTTask_OnMeleeAnimNoTarget()
{
	NodeName = TEXT("Task called when melee anim is playing without target");
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBotBTTask_OnMeleeAnimNoTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	return CoreTaskLogic(OwnerComp, NodeMemory);
}

void UBotBTTask_OnMeleeAnimNoTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	CoreTaskLogic(OwnerComp, NodeMemory);
}

EBTNodeResult::Type UBotBTTask_OnMeleeAnimNoTarget::CoreTaskLogic(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABotCharacter* Bot = Cast<ABotCharacter>(AIController->GetPawn());
	if (!Bot)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// 로직 처리
	if (!Bot->Server_GetIsBotMeleeAttacking())
	{
		AIController->GetBlackboardComponent()->SetValueAsBool(TEXT(AI_IS_MELEE_ANIM_PLAYING), false); // 밀리 애니메이션 재생 종료, 재생 종료 시점에 타깃이 유효하지 않을 경우
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}

	// 태스크 콜백 호출전까지 반복
	return EBTNodeResult::InProgress;
}

FString UBotBTTask_OnMeleeAnimNoTarget::GetStaticDescription() const
{
	return FString("The task will detect if melee animation is over, and if so change blackboard key and return success.");
}
