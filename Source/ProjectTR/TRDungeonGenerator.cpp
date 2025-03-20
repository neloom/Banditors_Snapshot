// Copyright (C) 2024 by Haguk Kim


#include "TRDungeonGenerator.h"
#include "Room.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreamingDynamic.h"
#include "DungeonGraph.h"
#include "TRMacros.h"
#include "ProjectTRGameModeBase.h"
#include "TRPlayerController.h"
#include "TRRoomObserverComponent.h"
#include "RoomLevel.h"
#include "RoomData.h"
#include "Door.h"
#include "DoorKeyActor.h"
#include "LockedDoorActor.h"
#include "ProceduralDungeonUtils.h"
#include "FPSCharacter.h"
#include "DoorBlocker.h"
#include "TRSpectatorPawn.h"
#include "SpawnPoint.h"

void ATRDungeonGenerator::BeginPlay()
{
	Super::BeginPlay();

	// 리프 룸 문 숫자가 2개 이상일 경우 무한루프에 빠질 수 있음
	check(LeafRoom);
	check(LeafRoom->GetNbDoor() <= 1);
	check(ExitingRoom);
	check(ExitingRoom->GetNbDoor() <= 1);

	MinimumRoomCount = FMath::Max(1, MinimumRoomCount);
	Generate();
}

void ATRDungeonGenerator::BlockInvalidDoor(const URoomData* Room, const FDoorDef& Door)
{
	if (!Room) return;

	// 빈 공간을 메꾼다
	FVector DoorPos = FDoorDef::GetRealDoorPosition(Door.Position, Door.Direction);
	FVector DoorDirection;
	switch (Door.Direction)
	{
	case EDoorDirection::North:
		DoorDirection = FVector::ForwardVector;
		break;
	case EDoorDirection::East:
		DoorDirection = FVector::RightVector;
		break;
	case EDoorDirection::South:
		DoorDirection = FVector::ForwardVector * -1;
		break;
	case EDoorDirection::West:
		DoorDirection = FVector::RightVector * -1;
		break;
	}
	//DrawDebugSphere(GetWorld(), DoorPos, 60, 2, FColor::Orange, true);
	FActorSpawnParameters DoorSpawnParam;
	DoorSpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<ADungeonActor>(DoorBlocker, FTransform(DoorDirection.Rotation(), DoorPos, FVector(1, 1, 1)), DoorSpawnParam);
	//TR_PRINT("OnFailedToAddRoomLogic");
}

void ATRDungeonGenerator::DestroyAllDoorBlockers()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADoorBlocker::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		ADoorBlocker* Blocker = Cast<ADoorBlocker>(Actor);
		if (Blocker)
		{
			Blocker->Destroy();
		}
	}
}

void ATRDungeonGenerator::Server_SetAllPlayerLocationBackToSpawn()
{
	if (!HasAuthority()) return;

	TR_PRINT("Server_SetAllPlayerLocationBackToSpawn");
	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (TRGM)
		{
			const TArray<ATRPlayerController*>& Players = TRGM->GetPlayersConnected();

			// 모든 플레이어를 최초 시작지점으로 이동시킨다
			for (ATRPlayerController* PC : Players)
			{
				if (!IsValid(PC)) continue;
				APawn* PlayerPawn = PC->GetPawnOrSpectator();
				if (!PlayerPawn) continue;

				AActor* PlayerStartActor = TRGM->FindPlayerStart(PC);
				if (PlayerStartActor)
				{
					PlayerPawn->SetActorLocation(PlayerStartActor->GetActorLocation());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Server_SetAllPlayerLocationBackToSpawn - Failed to find player start location(s). Aborting."));
				}
			}
		}
	}
}

void ATRDungeonGenerator::InitDungeonActorAfterGenComplete()
{
	const UDungeonGraph* DG = GetRooms();
	if (DG)
	{
		TArray<URoom*> CurrRooms = DG->GetAllRooms();
		for (URoom* CurrRoom : CurrRooms)
		{
			if (!CurrRoom) continue;
			TArray<AActor*> CurrRoomActors = GetAllActorsInRoom(CurrRoom);
			TArray<ASpawnPoint*> SpawnPoints;
			for (AActor* RoomActor : CurrRoomActors)
			{
				// 던전 액터 초기 설정 및 액터 틱 시작
				ADungeonActor* DungeonActor = Cast<ADungeonActor>(RoomActor);
				if (DungeonActor)
				{
					DungeonActor->OwningRoom = CurrRoom;
					DungeonActor->Initialize();
					ProcessDungeonActorDuringInit(DungeonActor);
				}

				// 스폰 포인트 기록
				ASpawnPoint* SpawnPoint = Cast<ASpawnPoint>(RoomActor);
				if (SpawnPoint)
				{
					SpawnPoints.Add(SpawnPoint);
				}
			}
			RoomSpawnPoints.Add(TPair<URoom*, TArray<ASpawnPoint*>>(CurrRoom, SpawnPoints));
		}
	}
}

void ATRDungeonGenerator::ProcessDungeonActorDuringInit(ADungeonActor* DungeonActor)
{
	// 필요 시 오버라이드 해서 작성
	return;
}

void ATRDungeonGenerator::Server_GenerateLockedDoors()
{
	if (!HasAuthority()) return;

	// NOTE: 잠긴 문과 관련된 모든 생성 로직은 이 하나의 패스 내에서 처리되도록 구현하는 것이 권장된다
	// 생성 시도 횟수 및 최대 생성 개수를 정한다
	int32 GenTrialCount = GetNbRoom();
	int32 MaxLockedPairCount = FMath::Clamp(GetNbRoom() / 15, TR_DUNGEON_MIN_LOCKED_DOORS_PER_LEVEL, TR_DUNGEON_MAX_LOCKED_DOORS_PER_LEVEL);

	// 개수가 적을수록 확률이 높게(원소 수가 많게) 설정
	TArray<int32> PairCountCandidates;
	PairCountCandidates.Add(0); // Empty 방지
	for (int32 Cnt = 0; Cnt <= MaxLockedPairCount; ++Cnt)
	{
		for (int32 i = 0; i < (MaxLockedPairCount - Cnt + 1) * 3; ++i) PairCountCandidates.Add(Cnt);
	}

	// 이번 패스에서 생성할 목표 페어 개수
	// 실제 생성 페어의 수는 이 값 이하가 된다
	int32 SelectedPairCount = PairCountCandidates[FMath::Rand() % PairCountCandidates.Num()];
	int32 PairsGenerated = 0;

	TArray<URoom*> RegularRooms = Graph->GetAllRooms();
	URoom* Entrance = GetRoomByIndex(0);
	URoom* Exit = ExitingRoomInst;
	if (!Entrance || !Exit || !RegularRooms.Contains(Entrance) || !RegularRooms.Contains(Exit))
	{
		UE_LOG(LogTemp, Error, TEXT("GenerateLockedDoors - Something went wrong!"));
		return;
	}
	RegularRooms.Remove(Entrance);
	RegularRooms.Remove(Exit);
	if (RegularRooms.Num() < 2) return;

	TArray<URoom*> KeyRooms;
	TArray<URoom*> LockedRooms;
	for (int32 TrialNum = 0; TrialNum < GenTrialCount; ++TrialNum)
	{
		if (RegularRooms.Num() < 2) break;

		// 키를 생성할 룸을 찾는다
		URoom* KeyRoom = Graph->GetRandomRoom(RegularRooms);
		RegularRooms.Remove(KeyRoom);
		URoom* LockedRoom = Graph->GetRandomRoom(RegularRooms);
		RegularRooms.Remove(LockedRoom);

		// 시작점 - 키 까지의 경로 사이에 잠긴 구간이 없어야 한다
		TArray<const URoom*> OutPath;
		bool bPathFound = Graph->FindPath(Entrance, KeyRoom, &OutPath, false/* TODO: check */);
		if (!bPathFound)
		{
			continue;
		}
		if (OutPath.Contains(LockedRoom))
		{
			// 순서가 뒤집힌 경우 스왑하면 경로가 유효해짐
			URoom* Temp = KeyRoom;
			KeyRoom = LockedRoom;
			LockedRoom = Temp;
		}

		// 키 및 잠긴 문을 생성한다
		// LockedRoom 노드의 어떤 문을 잠구던 상관 없음
		TArray<ADoor*> LockDoorCandidates;
		LockedRoom->GetAllDoors(LockDoorCandidates);
		if (LockDoorCandidates.IsEmpty())
		{
			continue;
		}
		ADoor* LockDoor = LockDoorCandidates[FMath::Rand() % LockDoorCandidates.Num()];
		if (!LockDoor || LockDoor->IsLocked() /* 두 번 잠글 수 없음*/)
		{
			continue;
		}
		if (!SpawnDoorKeyInstancePair(LockedRoom, KeyRoom, LockDoor))
		{
			continue;
		}

		// 페어 추가
		KeyRooms.Add(KeyRoom);
		LockedRooms.Add(LockedRoom);

		// 다음 페어 생성 시 경로 상에서 닫힌 노드로 취급하기 위해 Lock한다
		// NOTE: Lock 할 경우 해당 노드 및 노드의 모든 Door들이 Lock된다
		LockedRoom->Lock(true);

		PairsGenerated++;
		if (PairsGenerated >= SelectedPairCount) break;
	}

	// 모든 Lock을 다시 해제한다
	for (URoom* LockedRoom : LockedRooms)
	{
		LockedRoom->Lock(false);
	}

	/////TEMP
	UE_LOG(LogTemp, Error, TEXT("LockedPairGen: %d"), PairsGenerated);
}

bool ATRDungeonGenerator::SpawnDoorKeyInstancePair(URoom* DoorRoom, URoom* KeyRoom, ADoor* LockDoor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnDoorKeyInstancePair - Invalid world!"));
		return false;
	}
	if (!DoorRoom || !KeyRoom || !LockDoor)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnDoorKeyInstancePair - Invalid argument(s)!"));
		return false;
	}

	/*DrawDebugSphere(World, KeyRoom->GetBoundsCenter(), 100.0f, 16, FColor::Green, true, -1.0f, 0, 8.0f);
	DrawDebugSphere(World, LockDoor->GetActorLocation(), 100.0f, 16, FColor::Orange, true, -1.0f, 0, 8.0f);
	DrawDebugLine(World, KeyRoom->GetBoundsCenter(), DoorRoom->GetBoundsCenter(), FColor::White, true, -1.0f, 0, 8.0f);*/

	// 아이디
	int32 PairId = NewDoorKeyPairId;
	if (PairId < 0 || DoorKeyGenInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnDoorKeyInstancePair - Something went wrong!"));
		return false;
	}

	const FLockedDoorKeyPairInfo& PairGenInfo = DoorKeyGenInfos[FMath::Min(DoorKeyGenInfos.Num() - 1, PairId)];

	// 키 생성 위치 설정
	const TArray<ASpawnPoint*>& SpawnPoints = *RoomSpawnPoints.Find(KeyRoom);
	FVector SpawnLoc;
	bool bSpawnLocFound = TRUtils::GetRandomSpawnLocation(SpawnPoints, ESpawnPointType::SPT_ITEM_DOORKEY, SpawnLoc);
	if (!bSpawnLocFound)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnDoorKeyInstancePair - Unable to find valid key spawn point."));
		return false;
	}

	// 키 생성
	FActorSpawnParameters KeySpawnParam;
	KeySpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ADoorKeyActor* KeySpawned = World->SpawnActor<ADoorKeyActor>(
		PairGenInfo.DoorKeyClass,
		FTransform(FRotator(), SpawnLoc, FVector(1, 1, 1)),
		KeySpawnParam
	);
	if (!KeySpawned)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnDoorKeyInstancePair - Key spawn failed!"));
		return false;
	}
	KeySpawned->SetKeyId(PairId);

	// 문 생성
	FActorSpawnParameters DoorSpawnParam;
	DoorSpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALockedDoorActor* DoorSpawned = World->SpawnActor<ALockedDoorActor>(
		PairGenInfo.LockedDoorClass,
		FTransform(
			LockDoor->GetActorRotation(), 
			LockDoor->GetActorLocation() + LockDoor->GetActorRotation().RotateVector(PairGenInfo.DoorSpawnOffset), // NOTE: 오프셋이 필요한 이유는 문 액터가 정중앙을 기준으로 메시 정렬이 되있지 않기 때문임 (회전축을 문 끝쪽 모서리로 하기 위함)
			FVector(1, 1, 1)
		),
		DoorSpawnParam
	);
	if (!DoorSpawned)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnDoorKeyInstancePair - Door spawn failed!"));
		KeySpawned->SetCanBeDestroyed(true);
		KeySpawned->Destroy();
		return false;
	}
	DoorSpawned->SetDoorId(PairId);

	// 모든 절차가 완료되어야 아이디를 increment한다
	NewDoorKeyPairId++;
	return true;
}

URoomData* ATRDungeonGenerator::ChooseFirstRoomData_Implementation()
{
	return StartingRoom;
}

URoomData* ATRDungeonGenerator::ChooseNextRoomData_Implementation(const URoomData* CurrentRoom, const FDoorDef& DoorData, int& DoorIndex)
{
	if (Graph && Graph->Count() >= MinimumRoomCount)
	{
		if (bExitSpawned)
		{
			DoorIndex = 0;
			return LeafRoom;
		}
		else
		{
			DoorIndex = 0;
			return ExitingRoom;
		}
	}
	else
	{ 
		// 문 인덱스 번호는 빈 번호로 알아서 할당
		DoorIndex = -1;

		// 부모 노드 룸의 타입에 따라 연결될 수 있는 룸의 종류가 다름
		// 가령 lobby 타입의 룸 옆에는 또 다른 lobby 타입이 올 수 없음, 이는 부자연스럽기 때문임
		ETRRoomType CurrentRoomType = TryGetRoomType(CurrentRoom);
		TMap<URoomData*, int> Filtered = FilterRoomByParentType(CurrentRoomType);
		return GetRandomRoomDataWeighted(Filtered);
	}
	return nullptr;
}

TSubclassOf<class ADoor> ATRDungeonGenerator::ChooseDoor_Implementation(const URoomData* CurrentRoom, const URoomData* NextRoom, const UDoorType* DoorType, bool& Flipped)
{
	if (IsValid(CurrentRoom) && IsValid(NextRoom))
	{
		Flipped = false;
		return BaseDoorType;
	}
	return nullptr;
}

bool ATRDungeonGenerator::IsValidDungeon_Implementation()
{
	if (Graph && Graph->HasAlreadyRoomData(ExitingRoom))
	{
		return true;
	}
	return false;
}

bool ATRDungeonGenerator::ContinueToAddRoom_Implementation()
{
	// 계속 추가
	return true;
}

void ATRDungeonGenerator::InitializeDungeon_Implementation(const UDungeonGraph* Rooms)
{
}

void ATRDungeonGenerator::OnPreGeneration_Implementation()
{
}

void ATRDungeonGenerator::OnPostGeneration_Implementation()
{
	SetupExitingRoomInstance();
	InitDungeonActorAfterGenComplete();
	if (HasAuthority())
	{
		Server_GenerateLockedDoors(); // SetupExitingRoomInstance, InitDungeonActorAfterGenComplete 이후 호출
		Server_SetAllPlayerLocationBackToSpawn();
		Server_UpdateGameModeAfterGeneration();
	}
}

void ATRDungeonGenerator::OnGenerationInit_Implementation()
{
}

void ATRDungeonGenerator::OnGenerationFailed_Implementation()
{
}

void ATRDungeonGenerator::OnRoomAdded_Implementation(const URoomData* NewRoom)
{
	if (NewRoom == ExitingRoom)
	{
		bExitSpawned = true;
	}
}

void ATRDungeonGenerator::OnFailedToAddRoom_Implementation(const URoomData* FromRoom, const FDoorDef& FromDoor)
{
	if (HasAuthority())
	{
		BlockInvalidDoor(FromRoom, FromDoor);
	}
}

void ATRDungeonGenerator::OnInvalidDungeon_Implementation()
{
	if (HasAuthority())
	{
		DestroyAllDoorBlockers();
	}
	bExitSpawned = false;
}

APawn* ATRDungeonGenerator::GetVisibilityPawn_Implementation()
{
	APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!IsValid(Controller))
		return nullptr;
	APawn* LocalPawn = Controller->GetPawnOrSpectator();
	ATRSpectatorPawn* SpecPawn = Cast<ATRSpectatorPawn>(LocalPawn);
	if (SpecPawn)
	{
		APawn* Target = Cast<APawn>(SpecPawn->Local_GetSpectatingTarget());
		if (Target)
		{
			return Target;
		}
	}
	return LocalPawn;
}

void ATRDungeonGenerator::Server_UpdateGameModeAfterGeneration() const
{
	if (!HasAuthority()) return;
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_UpdateGameModeDuringGeneration - Invalid world!"));
		return;
	}
	AProjectTRGameModeBase* TRGM = World->GetAuthGameMode<AProjectTRGameModeBase>();
	if (!TRGM)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_UpdateGameModeDuringGeneration - Invalid game mode!"));
		return;
	}
	TRGM->bDungeonGenerationDataReceived = true;
	return;
}

TArray<AActor*> ATRDungeonGenerator::GetAllActorsInRoom(URoom* Room) const
{
	if (Room)
	{
		ULevel* RoomLevel = Room->Instance->GetLoadedLevel();
		if (RoomLevel)
		{
			return RoomLevel->Actors;
		}
	}
	return TArray<AActor*>();
}

URoom* ATRDungeonGenerator::Local_GetCurrentRoomOfLocalHost() const
{
	TArray<URoom*> DungeonRooms = Graph->GetAllRooms();
	for (URoom* Room : DungeonRooms)
	{
		// NOTE: 현재 위치에 따른 값을 갱신하는 로직은 Occulusion과 함께 처리된다
		if (Room->IsPlayerInside())
		{
			return Room;
		}
	}
	return nullptr;
}

TSet<URoom*> ATRDungeonGenerator::Server_GetPlayersRooms(AProjectTRGameModeBase* GameMode) const
{
	TSet<URoom*> PlayersRooms;
	PlayersRooms.Empty();
	const TArray<ATRPlayerController*>& PlayerControllers = GameMode->GetPlayersConnected();
	for (ATRPlayerController* Controller : PlayerControllers)
	{
		if (!IsValid(Controller))
		{
			UE_LOG(LogTemp, Error, TEXT("Server_GetRoomsOfOcculusionDepth - Player controller invalid! Skipping to next one."));
			continue;
		}

		// 관전 폰일 경우, 혹은 값이 없을 경우 스킵
		AFPSCharacter* Player = Cast<AFPSCharacter>(Controller->GetPawn());
		if (!IsValid(Player))
		{
			continue;
		}

		// 현재 속한 방 계산
		FBox WorldPlayerBox = Player->GetComponentsBoundingBox();
		FTransform Transform = UseGeneratorTransform() ? GetTransform() : FTransform::Identity;
		WorldPlayerBox = WorldPlayerBox.InverseTransformBy(Transform);

		FindElementsWithBoundsTest(*Octree, WorldPlayerBox, [this, &PlayersRooms](const FDungeonOctreeElement& Element)
			{
				PlayersRooms.Add(Element.Room);
			});
	}
	return PlayersRooms;
}

TSet<URoom*> ATRDungeonGenerator::Server_GetRoomsOfOcculusionDepth(int32 Depth)
{
	TSet<URoom*> Results;
	Results.Empty();
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_GetRoomsOfOcculusionDepth - Client should not call this function!"));
		return Results;
	}
	if (Depth < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_GetRoomsOfOcculusionDepth - Depth should be same or above 1!"));
		return Results;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (TRGM)
		{
			// 거리가 Depth 이하인 룸들만 반환
			GetRoomsWithinDistFrom(Server_GetPlayersRooms(TRGM), &Results, Depth);
			return Results;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Server_GetRoomsOfOcculusionDepth - Something went wrong!"));
	return Results;
}

TPair<TSet<URoom*>, TSet<URoom*>> ATRDungeonGenerator::Server_GetRoomsOfOcculusionDepthInBetween(int32 MinDepth, int32 MaxDepth)
{
	TSet<URoom*> Results;
	Results.Empty();
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_GetRoomsOfOcculusionDepth - Client should not call this function!"));
		return { Results, Results };
	}
	if (MinDepth < 1 || MaxDepth < 1 || MaxDepth < MinDepth)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_GetRoomsOfOcculusionDepth - Depth is Invalid!"));
		return { Results, Results };
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (TRGM)
		{
			TSet<URoom*> PlayersRooms = Server_GetPlayersRooms(TRGM);

			// 거리가 Min 이상이면서 Max 이하인 룸들을 구한다
			TSet<URoom*> InnerRooms;
			TSet<URoom*> OuterRooms;
			GetRoomsWithinDistFrom(PlayersRooms, &InnerRooms, MinDepth);
			GetRoomsWithinDistFrom(PlayersRooms, &OuterRooms, MaxDepth);

			Results = OuterRooms.Difference(InnerRooms);
			return { Results, OuterRooms };
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Server_GetRoomsOfOcculusionDepth - Something went wrong!"));
	return { Results, Results };
}

void ATRDungeonGenerator::GetRoomsWithinDistFrom(const TSet<URoom*>& InRooms, TSet<URoom*>* OutRooms, uint32 Distance)
{
	// 해시맵의 키 생성 (방 아이디, 순회 깊이)
	TArray<uint64> RoomIds = { UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX };
	for (URoom* InRoom : InRooms)
	{
		if (InRoom) RoomIds.Add(InRoom->GetRoomID());
	}
	RoomIds.Sort();
	FRoomMemoKey InRoomsKey = { RoomIds[0], RoomIds[1], RoomIds[2], RoomIds[3], RoomIds[4], RoomIds[5], RoomIds[6], RoomIds[7], Distance };

	if (!MemoizedResults.Contains(InRoomsKey) || !bDungeonGenerationComplete)
	{
		TSet<URoom*> NewOutRooms;
		UDungeonGraph::TraverseRooms(InRooms, &NewOutRooms, Distance, [](URoom* room) { /* 별도의 로직은 처리하지 않음 */ });

		// 던전 생성이 완료되지 않았다면 메모이제이션을 해서는 안된다
		if (!bDungeonGenerationComplete)
		{
			*OutRooms = TSet<URoom*>(NewOutRooms);
			return;
		}
		MemoizedResults.Add({ InRoomsKey, NewOutRooms });
	}
	*OutRooms = TSet<URoom*>(MemoizedResults[InRoomsKey]);
	return;
}

void ATRDungeonGenerator::SetupExitingRoomInstance()
{
	if (Graph)
	{
		TArray<URoom*> ExitRoom;
		Graph->GetAllRoomsFromData(ExitingRoom, ExitRoom);
		if (ExitRoom.Num() > 2)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupExitingRoomInstance - More than one exit has been generated!"));
			// NOTE: Exit은 오직 descend하는 포탈이 속한 방을 의미한다; 포탈이 여러 개 있더라도 그중 exit은 항상 하나 있어야 한다
		}
		else if (ExitRoom.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("SetupExitingRoomInstance - Cannot find any exit room instance! Aborting!"));
			return;
		}
		ExitingRoomInst = ExitRoom[0];
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("SetupExitingRoomInstance - Dungeon graph is invalid!"));
}

const uint32 ATRDungeonGenerator::Local_GetOcculusionDepth() const
{
	return Dungeon::OcclusionDistance();
}

TMap<class URoomData*, int> ATRDungeonGenerator::FilterRoomByParentType(ETRRoomType ParentType)
{
	TMap<class URoomData*, int> FilteredResult;
	for (TPair<URoomData*, int> Pair : DungeonRoomTypes)
	{
		if (CanBeAdjacentWith(ParentType, TryGetRoomType(Pair.Get<0>())))
		{
			FilteredResult.Add(Pair);
		}
	}
	return FilteredResult;
}

bool ATRDungeonGenerator::CanBeAdjacentWith(ETRRoomType ParentType, ETRRoomType ChildType) const
{
	// 로비에 로비를 맞닿게 생성할 수 없음
	if (ParentType == ETRRoomType::RT_Lobby && ChildType == ETRRoomType::RT_Lobby)
	{
		return false;
	}
	if (ParentType == ETRRoomType::RT_Passage && ChildType == ETRRoomType::RT_Passage)
	{
		return false;
	}
	return true;
}

ETRRoomType ATRDungeonGenerator::TryGetRoomType(const URoomData* Room) const
{
	ETRRoomType RoomType = ETRRoomType::RT_Default;

	bool bCustomDataFound = false;
	if (Room && !Room->CustomData.IsEmpty())
	{
		TSubclassOf<URoomCustomData> Data = Room->CustomData.Array()[0];
		UClass* TRCustomData = Data.Get();
		if (Data && TRCustomData)
		{
			bCustomDataFound = true;
			if (TRCustomData->IsChildOf<UTRRoomType_Default>()) RoomType = ETRRoomType::RT_Default;
			else if (TRCustomData->IsChildOf<UTRRoomType_Lobby>()) RoomType = ETRRoomType::RT_Lobby;
			else if (TRCustomData->IsChildOf<UTRRoomType_Passage>()) RoomType = ETRRoomType::RT_Passage;
			else if (TRCustomData->IsChildOf<UTRRoomType_Misc>()) RoomType = ETRRoomType::RT_Misc;
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TryGetRoomType - CustomData is found, but is unknown for room data %s!"), *(Room->GetName()));
			}
		}
	}

	if (!Room)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetRoomType - RoomData is invalid!"));
	}
	else if (!bCustomDataFound)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetRoomType - CustomData unavailable for room data %s"), *(Room->GetName()));
	}
	return RoomType;
}
