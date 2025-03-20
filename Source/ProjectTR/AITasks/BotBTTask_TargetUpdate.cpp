// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_TargetUpdate.h"
#include "BaseAIController.h"

UBotBTTask_TargetUpdate::UBotBTTask_TargetUpdate()
{
	NodeName = TEXT("Update target");
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBotBTTask_TargetUpdate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (AIController)
	{
		if (!AIController->UpdateCurrTargetValidity())
		{
			// 기존 타깃이 유효하지 않은 경우, 가능하다면 현재 퍼셉션된 액터들 중 새 타깃을 설정한다
			AIController->SetTargetWithinSight();
		}
	}
	if (AIController->GetCurrentTarget()) return EBTNodeResult::Succeeded;
	return EBTNodeResult::Failed;
}

FString UBotBTTask_TargetUpdate::GetStaticDescription() const
{
	return FString("Set new target from preperceived actors if target is invalid. Return failed if final target is null. Otherwise return success.");
}
