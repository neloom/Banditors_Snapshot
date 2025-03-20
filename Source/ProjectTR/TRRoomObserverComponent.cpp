// Copyright (C) 2024 by Haguk Kim


#include "TRRoomObserverComponent.h"
#include "RoomLevel.h"
#include "Room.h"
#include "DungeonGenerator.h"
#include "TRDungeonGenerator.h"
#include "TRMacros.h"
#include "GameCharacter.h"

UTRRoomObserverComponent::UTRRoomObserverComponent()
{
}

void UTRRoomObserverComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	// 감지 이벤트는 서버에서만 처리한다
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// 기본적으로 룸 옵저버 감지 로직은 공통 로직이다
		// 단 감지 성공 시의 게임플레이 로직은 대부분 서버에서 처리한다
		ActorEnterRoomEvent.AddDynamic(this, &UTRRoomObserverComponent::Server_ProcessActorEnterRoom);
		ActorExitRoomEvent.AddDynamic(this, &UTRRoomObserverComponent::Server_ProcessActorExitRoom);
	}
}

bool UTRRoomObserverComponent::IsValidRoomLevel(ARoomLevel* RoomLevel)
{
	if (!IsValid(RoomLevel) || !IsValid(RoomLevel->GetRoom()) || !Cast<ATRDungeonGenerator>(RoomLevel->GetRoom()->Generator()))
	{
		return false;
	}
	return true;
}

void UTRRoomObserverComponent::Server_ProcessActorEnterRoom(ARoomLevel* RoomLevel, AActor* Actor)
{
	if (!IsValidRoomLevel(RoomLevel))
	{
		UE_LOG(LogTemp, Error, TEXT("ProcessActorEnterRoom - RoomLevel is Invalid!"));
		return;
	}
	if (!Actor)
	{
		UE_LOG(LogTemp, Error, TEXT("ProcessActorEnterRoom - Actor is Invalid!"));
		return;
	}

	// 공통 로직 필요 시 작성

	AGameCharacter* GameCharacter = Cast<AGameCharacter>(Actor);
	if (GameCharacter)
	{
		Server_ProcessGameCharacterEnterRoom(RoomLevel, GameCharacter);
	}
	// else if ...
	// BaseItem 등 기타 액터 타입에 대한 전용 감지 처리가 필요할 경우 추가 가능
}

void UTRRoomObserverComponent::Server_ProcessActorExitRoom(ARoomLevel* RoomLevel, AActor* Actor)
{
	if (!IsValidRoomLevel(RoomLevel))
	{
		UE_LOG(LogTemp, Error, TEXT("ProcessActorEnterRoom - RoomLevel is Invalid!"));
		return;
	}
	if (!Actor)
	{
		UE_LOG(LogTemp, Error, TEXT("ProcessActorEnterRoom - Actor is Invalid!"));
		return;
	}

	// 공통 로직 필요 시 작성

	AGameCharacter* GameCharacter = Cast<AGameCharacter>(Actor);
	if (GameCharacter)
	{
		Server_ProcessGameCharacterExitRoom(RoomLevel, GameCharacter);
	}
	// else if ...
	// BaseItem 등 기타 액터 타입에 대한 전용 감지 처리가 필요할 경우 추가 가능
}

void UTRRoomObserverComponent::Server_ProcessGameCharacterEnterRoom(ARoomLevel* RoomLevel, AGameCharacter* Character)
{
	Character->Server_OnRoomEnter(RoomLevel);
}

void UTRRoomObserverComponent::Server_ProcessGameCharacterExitRoom(ARoomLevel* RoomLevel, AGameCharacter* Character)
{
	Character->Server_OnRoomExit(RoomLevel);
}
