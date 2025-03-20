// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "TRMacros.h"
#include "BaseAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class PROJECTTR_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	// 생성자
	ABaseAIController(const FObjectInitializer& ObjectInitializer);

	// 모든 게임 로직 관련 기능을 일시적으로 중지한다
	// Perception 정보와 컨트롤러 멤버 데이터는 손실되지만 블랙보드 값은 그대로 유지된다
	// 해당 데이터들의 경우 Deactivate된 상태에서 도중에 변동사항이 생겨 로직이 꼬일 우려가 있으므로 삭제하는 것이다
	// Deactivation과 같이 AI를 잠시 중지시키기 위한 경우 사용될 수 있으며,
	// Despawn과 같이 해당 컨트롤러를 완전히 리셋하려는 경우 HaltAILogic을 사용해야 한다
	void PauseAILogic();

	// AI의 기능을 정지하고 모든 내부 데이터를 완전히 초기화한다
	// 연산을 재개할 경우 초기 생성 상태와 동일하게 연산을 시작해야 한다
	void HaltAILogic();

	// 게임 관련 로직 연산을 재개한다
	void StartAILogic();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	
#pragma region /** Perception */
public:
	// 현재 타깃이 유효한지를 재확인해 상태를 업데이트한다
	// 만약 타깃이 더이상 유효하지 않다면 그에 맞는 로직을 실행한다
	// 현재 타깃의 유효성 여부를 반환한다
	bool UpdateCurrTargetValidity();

	// 현재 이미 시각 정보에 들어온 대상 액터들 중 가능한 타깃 하나를 설정한다
	// 새 타깃의 설정 여부를 반환한다
	// 이 함수는 기존 타깃이 시야 내에 있음에도 더이상 유효하지 않게 되었을 경우 
	// 시야 업데이트가 발생하지 않은 상태에서도 새 타깃을 설정하기 위해 사용된다
	bool SetTargetWithinSight();

	// 데미지를 입었을 경우 처리 로직
	// 공격자의 공격 시점 위치와 정보를 기록한다
	void OnDamageTaken(AActor* AttackedFrom, int32 Damage);

	// 타깃을 주어진 액터로 설정한다
	void SetTargetAs(AActor* Target);

	// 타깃을 해제한다 (nullptr로 설정한다)
	void ClearTarget();

	// 타깃의 마지막 목격 위치를 설정한다
	// NOTE: 정확히는 마지막 목격된 위치와 근접한 걸을 수 있는 네브메쉬 공간
	void SetTargetLastLocationOf(AActor* Target);
	
	// 타깃의 마지막 목격 정보를 초기화한다
	void ClearTargetLastLocation();

	// 공격자의 마지막 목격 위치를 설정한다
	void SetAttackerLastLocationAs(FVector Location);

	// 공격자 마지막 목격 정보를 초기화한다
	void ClearAttackerLastLocation();

	// 배회 목적지를 설정한다
	void SetPatrolLocationAs(FVector Location);

	// 배회 목적지 정보를 초기화한다
	void ClearPatrolLocation();

	// 타깃이나 Attacker 등 무언가에 포커스가 집중되었을 경우의 로직을 처리한다
	virtual void OnAggroStart();

	// 포커스가 해제되었을 경우의 로직을 처리한다
	virtual void OnAggroEnd();

	// Getter
	AActor* GetCurrentTarget() { return CurrentTarget; }

protected:
	// 인지 컴포넌트 이벤트
	UFUNCTION()
	void UpdatePerception(const TArray<AActor*>& Actors);

	UFUNCTION()
	void UpdateTargetPerception(AActor* Actor, FAIStimulus Stimulus);

	// 인지 정보가 업데이트 되었을 때의 로직을 처리한다
	virtual void OnPerceptionUpdate(const TArray<AActor*>& Actors);

	// 시각 정보로 타깃을 감지했을 때의 로직을 처리한다
	virtual void HandleTargetSightSense(AActor* Actor, FAIStimulus Stimulus);

	// 타깃이 유효한 공격 대상인지를 확인한다
	bool IsAttackTargetValid(AActor* Target);

	// 이 컨트롤러가 사용할 시각 정보에 대응되는 Sense ID를 반환한다
	virtual FAISenseID GetSightSenseID();

	// BT가 아닌 컨트롤러 단에서 저장되고 처리되는 정보들을 전부 초기 상태로 설정한다
	virtual void ClearControllerData();

	// 블랙보드에 저장된 모든 데이터를 초기화한다
	void ClearBlackboardValues();

protected:
	// 시각 정보
	// 필요 시 오버라이드 가능
	// NOTE: 이 값을 변경할 경우 블루프린트 컴포넌트 설정과 일치되도록 수동으로 조정할 필요가 있음
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class UAISense> SightSense = nullptr;
#pragma endregion

#pragma region /** Instance dependent variables*/
// NOTE:
// 인스턴스 개별 변수는 되도록 컨트롤러가 아닌, 캐릭터에 저장해야 한다
// 컨트롤러에 저장하게 되는 경우 ClearControllerData()에 해당 변수를 초기화하는 부분을 반드시 추가해 주어야 한다
protected:
	// 현재 추적중인 타깃
	AActor* CurrentTarget = nullptr;
#pragma endregion

protected:
	// BehaviorTree
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UBlackboardData> BlackboardAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset = nullptr;

	// 인지 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	TObjectPtr<class UAIPerceptionComponent> AIPerception = nullptr;

public:
	void PrintDebug() const;
};
