// Copyright (C) 2024 by Haguk Kim


#include "DungeonActor.h"
#include "Room.h"
#include "TRMacros.h"

ADungeonActor::ADungeonActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // 초기화 이후 틱이 활성화됨

	bReplicates = true;
	SetReplicatingMovement(true);

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	SetRootComponent(SceneComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetIsReplicated(true);
	MeshComponent->SetCollisionProfileName(TEXT("TRPhysicStatic"));
	MeshComponent->SetGenerateOverlapEvents(false); // 특수 액터의 경우 별도의 오버랩 설정이 필요
	MeshComponent->SetShouldUpdatePhysicsVolume(false);
}

void ADungeonActor::BeginPlay()
{
	Super::BeginPlay();

	// 던전 제너레이션 과정의 Initialize 호출이 불가능한 경우,
	// BeginPlay 시점에 수동으로 호출한다
	if (bForceInitDuringBeginPlay)
	{
		Initialize();
	}
}

void ADungeonActor::Initialize()
{
	if (bInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("Initialize - Called more than once! Please check bForceInitDuringBeginPlay. Aborting!"));
		return;
	}
	bInitialized = true;
	SetActorTickEnabled(true);
	ForceNetUpdate();
}

FVector ADungeonActor::ConvertRoomToWorld(const FVector RoomVector)
{
	if (!OwningRoom)
	{
		UE_LOG(LogTemp, Error, TEXT("ConvertRoomToWorld - Invalid owning room!"));
		return RoomVector;
	}
	return FVector(OwningRoom->RoomToWorld(FIntVector(RoomVector)));
}

void ADungeonActor::TriggerThis()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("DungeonActor logic should be handled server-only!"));
		return;
	}

	return OnTriggered();
}

void ADungeonActor::TriggerPairs()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("DungeonActor logic should be handled server-only!"));
		return;
	}

	for (int Index = 0; Index < Pairs.Num(); ++Index)
	{
		TriggerPair(Index);
	}
}

void ADungeonActor::TriggerPair(int Index)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("DungeonActor logic should be handled server-only!"));
		return;
	}

	if (Index < 0 || Index >= Pairs.Num() || !Pairs[Index].IsValid())
	{
		return;
	}
	Pairs[Index]->TriggerThis();
}

void ADungeonActor::AddPairActor(ADungeonActor* Pair)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("DungeonActor logic should be handled server-only!"));
		return;
	}

	Pairs.Add(MakeWeakObjectPtr<ADungeonActor>(Pair));
}

void ADungeonActor::OnTriggered()
{
	// 테스트 코드; 필요 시 상속해 오버라이드 할 것 
	TR_PRINT_FSTRING("%s", *GetName());
	TriggerPairs();
	return;
}

