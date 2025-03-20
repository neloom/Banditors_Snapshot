// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "DungeonGenerator.h"
#include "DungeonActor.h"
#include "TRRoomCustomData.h"
#include "TRMacros.h"
#include "TRDungeonGenerator.generated.h"

USTRUCT()
struct FRoomMemoKey
{
	GENERATED_BODY()

	// NOTE:
	// 이 구조체는 GetRoomsWithinDistFrom의 메모이제이션에 사용할 키값으로 사용된다.
	// Room1~8는 InRooms에 포함되는 방들의 고유 id값들이다
	// 최대 4인 플레이가 가능하므로 일반적으로 4개의 방을 기준으로 메모이제이션을 기록한다
	// 이때 하나의 플레이어가 동시에 두 방에 겹쳐져 있을 수 있는데, 이 경우 8개까지 선택될 수 있다 (굉장히 특수한 경우 2개 초과해서 겹칠 수도 있지만, 이 경우는 생각하지 않는다)
	// 각 방들의 id를 오름차순으로 정렬해 키값으로 사용한다
	// 원래 인풋으로 제공되는 TSet 자체를 키값으로 사용하려 했으나, 원소의 개수가 적기 때문에 보다 빠른 이 방식을 사용한다
	// 방들의 순서는 플레이어 순서와 전혀 무관하다
	// NULL값은 UINT64_MAX로 나타낸다

	FRoomMemoKey() : Room1(UINT64_MAX), Room2(UINT64_MAX), Room3(UINT64_MAX), Room4(UINT64_MAX), Room5(UINT64_MAX), Room6(UINT64_MAX), Room7(UINT64_MAX), Room8(UINT64_MAX), Depth(1) {}
	FRoomMemoKey(uint64 R1, uint64 R2, uint64 R3, uint64 R4, uint64 R5, uint64 R6, uint64 R7, uint64 R8, uint32 InDepth) : Room1(R1), Room2(R2), Room3(R3), Room4(R4), Room5(R5), Room6(R6), Room7(R7), Room8(R8), Depth(InDepth) {}

	UPROPERTY()
	uint64 Room1;

	UPROPERTY()
	uint64 Room2;

	UPROPERTY()
	uint64 Room3;

	UPROPERTY()
	uint64 Room4;

	UPROPERTY()
	uint64 Room5;

	UPROPERTY()
	uint64 Room6;

	UPROPERTY()
	uint64 Room7;

	UPROPERTY()
	uint64 Room8;

	UPROPERTY()
	uint32 Depth;

	bool operator==(const FRoomMemoKey& Other) const
	{
		return Room1 == Other.Room1 && Room2 == Other.Room2 && Room3 == Other.Room3 && Room4 == Other.Room4 && Room5 == Other.Room5 && Room6 == Other.Room6 && Room7 == Other.Room7 && Room8 == Other.Room8 && Depth == Other.Depth;
	}

	friend uint32 GetTypeHash(const FRoomMemoKey& In)
	{
		return FCrc::MemCrc32(&In, sizeof(FRoomMemoKey));
	}
};


USTRUCT(BlueprintType)
struct FLockedDoorKeyPairInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ALockedDoorActor> LockedDoorClass = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ADoorKeyActor> DoorKeyClass = nullptr;

	UPROPERTY(EditAnywhere)
	UTexture2D* DoorKeyTexture = nullptr;

	UPROPERTY(EditAnywhere)
	FVector DoorSpawnOffset = FVector::ZeroVector;
};


/**
 * 
 */
UCLASS()
class PROJECTTR_API ATRDungeonGenerator : public ADungeonGenerator
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

#pragma region /** Dungeon generation */
protected:
	// 방 생성 실패 시 해당 위치를 벽으로 가로막는다
	void BlockInvalidDoor(const URoomData* Room, const FDoorDef& Door);

	// 월드에 존재하는 모든 DoorBlocker들을 제거한다
	// 던전 생성을 여러 차례 시도하기 때문에, 각 시행이 끝난 후 생성에 실패했다면 DoorBlocker를 제거해주어야 한다
	void DestroyAllDoorBlockers();

	// 모든 플레이어를 최초 스폰 지점으로 돌려보낸다
	void Server_SetAllPlayerLocationBackToSpawn();

	// 던전 생성이 끝난 이후 던전 내 동적으로 생성된 모든 액터들에 대해 처리해주어야 하는 로직들을 처리한다
	void InitDungeonActorAfterGenComplete();

	// 초기화 시 개별 액터 단위로 처리해주어야 하는 로직
	virtual void ProcessDungeonActorDuringInit(class ADungeonActor* DungeonActor);

	// 잠긴 문과 그 문을 개방할 수 있는 개방 장치를 짝지어 생성한다
	void Server_GenerateLockedDoors();

	// 새 문 - 키 페어 인스턴스를 생성한다
	// 성공 여부를 반환한다
	bool SpawnDoorKeyInstancePair(URoom* DoorRoom, URoom* KeyRoom, ADoor* LockDoor);

protected:
	// 던전 생성 로직 오버라이드
	virtual class URoomData* ChooseFirstRoomData_Implementation();
	virtual class URoomData* ChooseNextRoomData_Implementation(const URoomData* CurrentRoom, const FDoorDef& DoorData, int& DoorIndex);
	virtual TSubclassOf<class ADoor> ChooseDoor_Implementation(const URoomData* CurrentRoom, const URoomData* NextRoom, const UDoorType* DoorType, bool& Flipped);
	virtual bool IsValidDungeon_Implementation();
	virtual bool ContinueToAddRoom_Implementation();
	virtual void InitializeDungeon_Implementation(const class UDungeonGraph* Rooms);
	virtual void OnPreGeneration_Implementation();
	virtual void OnPostGeneration_Implementation();
	virtual void OnGenerationInit_Implementation();
	virtual void OnGenerationFailed_Implementation();
	virtual void OnRoomAdded_Implementation(const URoomData* NewRoom);
	virtual void OnFailedToAddRoom_Implementation(const URoomData* FromRoom, const FDoorDef& FromDoor);
	virtual void OnInvalidDungeon_Implementation();

	// 기타 오버라이드
	virtual APawn* GetVisibilityPawn_Implementation();

public:
	// 던전 생성이 완전히 완료된 이후 게임모드에 어떤 특정 값을 전달해야 하는 경우 여기서 처리한다
	// NOTE: 가장 하위의 자손 함수부터 먼저 처리되며, 이는 모든 과정이 성공적일 경우에만 bDungeonGenerationComplete를 마지막에 true로 설정하기 위함이다
	// 즉 자손 함수 실행에 실패했다면 부모 함수는 실행되지 않는다
	virtual void Server_UpdateGameModeAfterGeneration() const;

	// Getter/Setter
	FORCEINLINE bool IsDungeonGenerationComplete() { return bDungeonGenerationComplete; }
	FORCEINLINE void UpdateDungeonGenerationState() { bDungeonGenerationComplete = GetProgress() >= 1.0f; }

private:
	// 던전의 생성 여부; 이 값에 대한 연산은 getter/setter로만 처리해야 한다
	bool bDungeonGenerationComplete = false;

protected:
	// 던전 최소 방 개수
	UPROPERTY(EditDefaultsOnly)
	int32 MinimumRoomCount = 1;

	// 방 생성 실패 시 문 위치에 생성할 액터
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ADungeonActor> DoorBlocker = nullptr;

	// 잠긴 문 - 열쇠 페어 생성에 사용할 정보
	// 페어 ID번 인덱스 정보를 사용한다
	// 만약 ID가 인덱스 수를 초과하는 경우 가장 마지막 인덱스를 사용한다
	UPROPERTY(EditDefaultsOnly)
	TArray<FLockedDoorKeyPairInfo> DoorKeyGenInfos;

	// 던전 생성에 사용되는 방들의 종류
	UPROPERTY(EditDefaultsOnly)
	TMap<class URoomData*, int> DungeonRoomTypes;

	// 초기 루트 노드로 사용할 방
	UPROPERTY(EditDefaultsOnly)
	class URoomData* StartingRoom = nullptr;

	// 다음 층계로 이동하기 위해 도착해야 하는 방
	UPROPERTY(EditDefaultsOnly)
	class URoomData* ExitingRoom = nullptr;
	class URoom* ExitingRoomInst = nullptr;

	// 노드 끝(리프노드)에 사용할 방
	// TODO: 후보 여러개 사용 가능하도록 개선 필요
	UPROPERTY(EditDefaultsOnly)
	class URoomData* LeafRoom = nullptr;

	// 현재 던전에 출구가 생성되었는지 여부
	bool bExitSpawned = false;

	// TODO
	// 기본 문 타입
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ADoor> BaseDoorType = nullptr;

private:
	// 새 페어를 생성할 때 이 값을 토대로 issue한다
	int32 NewDoorKeyPairId = 0;

public:
	// 각 방별로 할당된 스폰 포인트
	// 스폰 포인트가 없을 경우 빈 어레이가 할당될 수 있다
	TMap<URoom*, TArray<class ASpawnPoint*>> RoomSpawnPoints;

/* Utility */
public:
	// 룸 내에 속한 모든 액터를 반환한다
	UFUNCTION(BlueprintCallable)
	TArray<AActor*> GetAllActorsInRoom(class URoom* Room) const;

	// 현재 로컬 플레이어가 속한 룸을 반환한다
	UFUNCTION(BlueprintCallable)
	URoom* Local_GetCurrentRoomOfLocalHost() const;

	// 모든 플레이어들이 현재 속해있는 방들의 목록을 반환한다
	TSet<URoom*> Server_GetPlayersRooms(class AProjectTRGameModeBase* GameMode) const;

	// 각 플레이어들이 속한 방을 기준으로 Depth 이하의 상대적 깊이에 속하는 방들의 목록을 반환한다
	TSet<URoom*> Server_GetRoomsOfOcculusionDepth(int32 Depth);

	// 각 플레이어들이 속한 방을 기준으로 MinDepth 이상 MaxDepth 이하의 상대적 깊이에 속하는 방들의 목록을 반환한다
	// 첫 반환값은 Min이상 Max이하의 방들의 목록이고, 두번째 반환값은 Max이하 전체 방들의 목록이다
	TPair<TSet<URoom*>, TSet<URoom*>> Server_GetRoomsOfOcculusionDepthInBetween(int32 MinDepth, int32 MaxDepth);

	// 로컬에서 사용중인 Occulusion depth 값을 반환한다
	const uint32 Local_GetOcculusionDepth() const;

	// 부모 방의 Type을 기준으로 해당 방에 연결될 수 있는 방들만 필터링해 반환한다
	TMap<class URoomData*, int> FilterRoomByParentType(ETRRoomType ParentType);

	// 두 룸 타입이 서로 맞붙어있을 수 있는지를 반환한다
	bool CanBeAdjacentWith(ETRRoomType ParentType, ETRRoomType ChildType) const;
	
	// 주어진 룸 데이터 내에 커스텀 데이터가 포함되어있는지를 확인하고 추정되는 룸 타입을 반환한다
	// 커스텀 데이터가 없을 경우 정확하게 동작하지 않을 수 있다
	ETRRoomType TryGetRoomType(const class URoomData* Room) const;

/* Dungeon graph */
public:
	// UDungeonGraph::TraverseRooms의 메모이제이션을 통해 최적화된 버전의 함수
	// InRooms에 대해 Distance 내에 속한 방들을 반환한다
	// 같은 인풋이 들어오는 경우가 매우 잦기 때문에 성능 개선에 많은 도움이 된다
	void GetRoomsWithinDistFrom(const TSet<URoom*>& InRooms, TSet<URoom*>* OutRooms, uint32 Distance);

protected:
	// 던전 생성 완료 후 호출되며, 그래프에서 exit room을 찾아 그 참조를 기록한다
	void SetupExitingRoomInstance();

protected:
	// 실행 결과 메모이제이션
	TMap<FRoomMemoKey, TSet<URoom*>> MemoizedResults;
#pragma endregion
};
