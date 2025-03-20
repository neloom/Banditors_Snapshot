// Copyright (C) 2024 by Haguk Kim


#include "AIControllerPool.h"
#include "BaseAIController.h"
#include "BotCharacter.h"
#include "BrainComponent.h"

UAIControllerPool::UAIControllerPool()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ABaseAIController* UAIControllerPool::CreateNewBaseAIController()
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (!AIControllerClass)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateNewBaseAIController - AIControllClass is null!"));
			return nullptr;
		}
		ABaseAIController* BaseController = World->SpawnActor<ABaseAIController>(AIControllerClass);
		if (BaseController)
		{
			// NOTE: AI컨트롤러 풀링 시 반드시 BrainComponent의 로직을 중지시켜야 크래시가 발생하지 않는다
			BaseController->HaltAILogic();
			return BaseController;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("CreateNewBaseAIController - World pointer is invalid!"));
	return nullptr;
}

int32 UAIControllerPool::GetAnyElementFromSet(TSet<int32>& Set)
{
	if (Set.Num() > 0)
	{
		TSet<int32>::TConstIterator It(Set);
		return *It;
	}
	return -1;
}

void UAIControllerPool::BeginPlay()
{
	Super::BeginPlay();
	
}

ABaseAIController* UAIControllerPool::Animate(ABotCharacter* Bot, bool bForceNewController)
{
	if (!IsValid(Bot))
	{
		UE_LOG(LogTemp, Error, TEXT("UAIControllerPool::Animate - Bot is Invalid."));
		return nullptr;
	}

	ABaseAIController* NewController = nullptr;
	if (bForceNewController)
	{
		NewController = CreateNewBaseAIController();
	}
	else
	{
		int32 ControllerIndex = GetAnyElementFromSet(ValidIndexes);
		if (ControllerIndex >= 0 && ControllerIndex < Pool.Num())
		{
			NewController = Pool[ControllerIndex];
			Remove(ControllerIndex, false);
		}
		if (!NewController)
		{
			UE_LOG(LogTemp, Error, TEXT("UAIControllerPool::Animate - Controller is Invalid. Creating new one!"));
			NewController = CreateNewBaseAIController();
		}
	}

	if (NewController)
	{
		NewController->Possess(Bot);
		NewController->StartAILogic();
		//NewController->PrintDebug();
		return NewController;
	}
	UE_LOG(LogTemp, Error, TEXT("UAIControllerPool::Animate - Something went wrong!"));
	return nullptr;
}

void UAIControllerPool::Inanimate(ABotCharacter* Bot)
{
	if (!IsValid(Bot))
	{
		UE_LOG(LogTemp, Error, TEXT("UAIControllerPool::Inanimate - Bot is Invalid."));
		return;
	}
	ABaseAIController* AIController = Cast<ABaseAIController>(Bot->GetController());
	if (!IsValid(AIController))
	{
		UE_LOG(LogTemp, Error, TEXT("UAIControllerPool::Inanimate - Bot's controller is Invalid. IsNull: %d"), (AIController == nullptr));
		return;
	}

	AIController->HaltAILogic();
	AIController->UnPossess();

	int32 PutIndex = GetAnyElementFromSet(EmptyIndexes);
	if (PutIndex >= 0 && PutIndex < Pool.Num())
	{
		Put(AIController, PutIndex);
	}
	else
	{
		Insert(AIController);
	}
	return;
}

void UAIControllerPool::Initialize(int32 PoolSize)
{
	Pool.Reserve(PoolSize);
	for (int32 EmptyIndex = 0; EmptyIndex < PoolSize; ++EmptyIndex)
	{
		EmptyIndexes.Add(EmptyIndex);
	}

	for (int Index = 0; Index < PoolSize; ++Index)
	{
		ABaseAIController* AIController = CreateNewBaseAIController();
		if (!AIController)
		{
			UE_LOG(LogTemp, Error, TEXT("UAIControllerPool::Fill - Bot's controller is Invalid. IsNull: %d"), (AIController == nullptr));
			return;
		}
		Insert(AIController);
	}
}

void UAIControllerPool::Put(ABaseAIController* Controller, int32 Index)
{
	if (!IsValid(Controller))
	{
		UE_LOG(LogTemp, Warning, TEXT("UAIControllerPool::Put - Controller is invalid. Consider using Remove() instead."));
		return;
	}
	if (Index < 0 || Index >= Pool.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UAIControllerPool::Put - Index is out of bounds. Adding new element instead. Consider using Insert() instead."));
		return;
	}
	EmptyIndexes.Remove(Index);
	ValidIndexes.Add(Index);

	// 덮어쓴다
	Pool[Index] = Controller;
}

int32 UAIControllerPool::Insert(ABaseAIController* Controller)
{
	int32 InsertIndex = Pool.Add(Controller);
	EmptyIndexes.Remove(InsertIndex);
	ValidIndexes.Add(InsertIndex);
	return InsertIndex;
}

void UAIControllerPool::Remove(int32 Index, bool bDestroy)
{
	if (Pool[Index] && bDestroy)
	{
		Pool[Index]->Destroy();
	}
	ValidIndexes.Remove(Index);
	EmptyIndexes.Add(Index);
	Pool[Index] = nullptr;
}
