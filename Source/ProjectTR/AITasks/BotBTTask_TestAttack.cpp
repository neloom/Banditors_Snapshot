// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_TestAttack.h"
#include "AIController.h"
#include "BotCharacter.h"
#include "TRMacros.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_TestAttack::UBotBTTask_TestAttack()
{
	NodeName = TEXT("Temporary task for bot attack");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBotBTTask_TestAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABotCharacter* Bot = Cast<ABotCharacter>(AIController->GetPawn());
	if (!Bot)
	{
		return EBTNodeResult::Failed;
	}

	// 로직 처리
	if (!Bot->Server_GetIsBotMeleeAttacking())
	{
		AIController->GetBlackboardComponent()->SetValueAsBool(TEXT(AI_IS_MELEE_ANIM_PLAYING), true); // 밀리 애니메이션 재생 시작
		Bot->Server_BotMeleeAttack();
		bIsAttackPendingFinish = true;
	}

	// 공격 Register 즉시 처리 완료된 것으로 가정 (밀리 로직은 별도로 ANS에서 처리되기 때문에, 다시 곧바로 추격 재개)
	return EBTNodeResult::Succeeded;
}

void UBotBTTask_TestAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	ABotCharacter* Bot = Cast<ABotCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (bIsAttackPendingFinish && !Bot->Server_GetIsBotMeleeAttacking())
	{
		bIsAttackPendingFinish = false;
		AAIController* AIController = OwnerComp.GetAIOwner();
		if (AIController)
		{
			AIController->GetBlackboardComponent()->SetValueAsBool(TEXT(AI_IS_MELEE_ANIM_PLAYING), false); // 밀리 애니메이션 재생 종료, 종료 시점에도 타깃이 유효한 경우
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

FString UBotBTTask_TestAttack::GetStaticDescription() const
{
	return FString();
}
