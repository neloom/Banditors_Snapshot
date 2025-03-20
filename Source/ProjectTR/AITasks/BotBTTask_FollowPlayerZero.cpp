// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_FollowPlayerZero.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBotBTTask_FollowPlayerZero::UBotBTTask_FollowPlayerZero()
{
	NodeName = TEXT("Setup destination to the location of player zero");

	// 벡터로 자료형 한정
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBotBTTask_FollowPlayerZero, BlackboardKey)/*ASSERTION*/);
}

EBTNodeResult::Type UBotBTTask_FollowPlayerZero::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	FNavLocation Location;

	// 폰 획득
	AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* AIPawn = AIController->GetPawn();

	// 0번 플레이어 위치 선택
	FVector PlayerZeroLocation;
	const UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			APawn* PlayerPawn = PlayerController->GetPawn();
			if (PlayerPawn)
			{
				PlayerZeroLocation = PlayerPawn->GetActorLocation();
				AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, PlayerZeroLocation);
				//DrawDebugSphere(World, PlayerZeroLocation, 10, 10, FColor::Green, false, 0.5f);
			}
		}
	}

	// 태스크 종료
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

FString UBotBTTask_FollowPlayerZero::GetStaticDescription() const
{
	return FString::Printf(TEXT("Vector: %s"), *BlackboardKey.SelectedKeyName.ToString());
}
