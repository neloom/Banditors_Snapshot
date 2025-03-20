// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIControllerPool.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTTR_API UAIControllerPool : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIControllerPool();

	// 인자로 주어진 봇에게 풀에서 컨트롤러 하나를 할당해 Possess한다
	class ABaseAIController* Animate(class ABotCharacter* Bot, bool bForceNewController = false);

	// 봇에 할당된 컨트롤러를 Unpossess해 오브젝트풀로 반환한다
	void Inanimate(class ABotCharacter* Bot);

	// 풀을 초기화한다
	void Initialize(int32 PoolSize);

protected:
	// 특정 위치에 주어진 컨트롤러를 할당한다
	void Put(class ABaseAIController* Controller, int32 Index);

	// 가장 끝에 새 컨트롤러를 할당한다
	// 추가 후 추가된 인덱스를 반환한다
	int32 Insert(class ABaseAIController* Controller);

	// 특정 위치의 값을 삭제한다
	// bDestroy가 true일 경우, 해당 위치에 할당된 값이 있을 경우 해제한다
	void Remove(int32 Index, bool bDestroy = false);

	// AIControllerClass를 기반으로 새 AI 컨트롤러를 생성한다
	// 비활성화 상태로 생성한다
	class ABaseAIController* CreateNewBaseAIController();

	// 순서 상관 없이 아무 원소나 하나를 반환한다
	// Set이 비어있을 경우 -1을 반환한다
	int32 GetAnyElementFromSet(TSet<int32>& Set);
protected:
	virtual void BeginPlay() override;

protected:
	// 오브젝트 풀
	// Put()과 Remove(), Insert()를 제외하면 Pool에 직접적인 Write연산을 가해서는 안된다
	TArray<class ABaseAIController*> Pool;

	// 풀 어레이의 공간들 중 비어있는 인덱스들을 관리한다
	TSet<int32> EmptyIndexes;

	// 비어있지 않은 인덱스들
	TSet<int32> ValidIndexes;

	// AI 풀에 사용할 컨트롤러 클래스
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABaseAIController> AIControllerClass;
};
