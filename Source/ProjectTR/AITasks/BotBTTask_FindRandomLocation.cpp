// Copyright (C) 2024 by Haguk Kim


#include "BotBTTask_FindRandomLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TRMacros.h"

UBotBTTask_FindRandomLocation::UBotBTTask_FindRandomLocation()
{
	NodeName = TEXT("Find Random Location Within Radius");
}

EBTNodeResult::Type UBotBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	FNavLocation Location;

	// 폰 획득
	AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* AIPawn = AIController->GetPawn();

	// 범위 내 랜덤한 위치 선택
	FVector2D CurrOffset = FVector2D::ZeroVector;
	if (!SearchOffsets.IsEmpty())
	{
		CurrOffset = SearchOffsets[FMath::Rand() % SearchOffsets.Num()];
	}
	const FVector& Origin = AIPawn->GetActorLocation() + (AIPawn->GetActorForwardVector() * CurrOffset.Y) + (AIPawn->GetActorRightVector() * CurrOffset.X);
	const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (IsValid(NavSystem) && NavSystem->GetRandomPointInNavigableRadius(Origin, SearchRadius, Location))
	{
		AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, Location.Location);
		
		DrawDebugSphere(GetWorld(), Location.Location, 10, 10, FColor::Red, false, 0.5f);
	}

	// 태스크 종료
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

FString UBotBTTask_FindRandomLocation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Vector: %s"), *BlackboardKey.SelectedKeyName.ToString());
}
