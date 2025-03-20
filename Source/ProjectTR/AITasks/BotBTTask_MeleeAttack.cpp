// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_MeleeAttack.h"
#include "BaseAIController.h"
#include "BotCharacter.h"
#include "TRMacros.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_MeleeAttack::UBotBTTask_MeleeAttack()
{
	NodeName = TEXT("Melee attack task for bots");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true; // 액터별로 프로퍼티 값이 달라질 수 있음
}

EBTNodeResult::Type UBotBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	ABotCharacter* Bot = Cast<ABotCharacter>(AIController->GetPawn());
	if (!Bot || !AIController)
	{
		return EBTNodeResult::Failed;
	}

	// 로직 처리
	if (!Bot->Server_GetIsBotMeleeAttacking())
	{
		if (ShouldTryAttack(Bot, AIController->GetCurrentTarget()))
		{
			AIController->GetBlackboardComponent()->SetValueAsBool(TEXT(AI_IS_MELEE_ANIM_PLAYING), true); // 밀리 애니메이션 재생 시작
			Bot->Server_BotMeleeAttack();
			bIsAttackPendingFinish = true;
		}
	}
	return EBTNodeResult::Succeeded;
}

void UBotBTTask_MeleeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

FString UBotBTTask_MeleeAttack::GetStaticDescription() const
{
	return FString("Performs a animation-based melee attack.");
}

bool UBotBTTask_MeleeAttack::ShouldTryAttack(ABotCharacter* Bot, AActor* Target)
{
	return Bot && (!bShouldCheckAtkTrialRange || bShouldCheckAtkTrialRange && Target && Bot->IsTargetInMeleeAtkTrialRange(Target));
}

