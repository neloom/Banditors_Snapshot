// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BotBTTask_AimAtTarget.generated.h"

/**
 * 액터 Yaw 값을 조정해 타깃을 바라보도록 한다
 */
UCLASS()
class PROJECTTR_API UBotBTTask_AimAtTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
	UBotBTTask_AimAtTarget();

protected:
	// 실행부
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 틱
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// BP node description
	virtual FString GetStaticDescription() const override;

private:
	// 초기화 여부
	bool bInitialized = false;

	// 각도 오차 허용치
	float Precision = 1.0f;

	// 회전 각속도
	float YawRotateSpeed = 2.0f;

	// LOS 체크 여부
	// 장애물에 막히지 않고 타깃을 향해야 하는 경우 true로 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Line of sight checking", meta = (AllowPrivateAccess = true))
	bool bShouldCheckLOS = false;
};
