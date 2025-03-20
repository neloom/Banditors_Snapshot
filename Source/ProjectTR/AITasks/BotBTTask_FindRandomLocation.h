// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BotBTTask_FindRandomLocation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UBotBTTask_FindRandomLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBotBTTask_FindRandomLocation();

private:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// BP node description
	virtual FString GetStaticDescription() const override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Radius", meta = (AllowPrivateAccess = true))
	float SearchRadius = 2000.0f;

	// AI 오너 전후좌우축을 기준 얼마만큼의 오프셋을 서치 원의 중심으로 삼을지 결정
	// 값이 여러 개 있을 경우 랜덤으로 하나를 선택
	// X는 우측, Y는 전방이 양의 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Offsets (Local)", meta = (AllowPrivateAccess = true))
	TArray<FVector2D> SearchOffsets;
};
