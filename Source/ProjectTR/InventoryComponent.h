// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponent.h"
#include "TRMacros.h"
#include "Engine/ActorChannel.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInvObjData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 GridX = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 GridY = 0;
	
	// Struct atomic replication
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << GridX;
		Ar << GridY;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FInvObjData> : public TStructOpsTypeTraitsBase2<FInvObjData>
{
	enum
	{
		WithNetSerializer = true
	};
};

UCLASS( ClassGroup=(Custom), BlueprintType, meta=(BlueprintSpawnableComponent) )
class PROJECTTR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 기존 값을 삭제하고 필요한 초기화를 진행한다
	// 이 함수는 최초 호출 시에만 유효하게 동작하고 이후 중복 호출 시에는 아무 것도 처리하지 않는다
	void Initialize();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 캐싱에 사용할 인벤토리 정보를 추출해 외부에서 사용할 수 있도록 반환한다
	// 내부 정보를 밖으로 이동시키는 행위이기 때문에, 더이상 해당 컴포넌트 인스턴스의 사용이 필요하지 않은 경우에만 사용해야 한다
	// 인벤토리의 복구에 필요한 필수 데이터들에 한해서만 정보를 추출한다
	// Keys의 경우 인벤토리를 복구하는 과정에서 새로 할당되기 때문에 전달이 불필요하다
	void Server_ExportInventoryInfo(TArray<class UInvObject*>* OutValues, TArray<FInvObjData>* OutDatas);

	// 인벤토리 정보를 기반으로 컴포넌트를 복구한다
	void Server_ImportInvComp(TArray<class UInvObject*> NewValues, TArray<FInvObjData> NewDatas);

protected:
	// 현재 이 컴포넌트가 Importing 작업을 수행중인지 여부
	// 인벤토리가 아직 importing 중이라는 의미는 아직 완전히 게임플레이 로직을 처리할 준비가 되지 않았다는 것을 의미한다
	// 즉 일부 게임플레이 로직의 처리를 Import 단계에서 처리하는 것을 막기 위해 사용된다
	UPROPERTY(BlueprintReadOnly)
	bool bIsImportingInventory = false;

#pragma region /** Networking */
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME(UInventoryComponent, InvObjectKeys);
		DOREPLIFETIME(UInventoryComponent, InvObjectValues);
		DOREPLIFETIME(UInventoryComponent, InvObjectDatas);
		DOREPLIFETIME(UInventoryComponent, Grid);
		DOREPLIFETIME(UInventoryComponent, RefreshCount);
	}

	// 아이템 추가 RPC
	UFUNCTION(Server, Reliable)
	void Server_AddInvObjectRPC(class UInvObject* InvObj, int32 TopLeftX, int32 TopLeftY);

	// 아이템 제거 RPC
	UFUNCTION(Server, Reliable)
	void Server_RemoveInvObjectRPC(int32 ObjId);

	// 아이템 스폰 (InvObject 기반 생성) RPC
	UFUNCTION(Server, Reliable)
	void Server_SpawnItemFromInvObjectRPC(class UInvObject* InvObj, FVector Location);

	// 아이템을 이동하기 위해 호출하는 RPC
	// 동일 컴포넌트 내의 이동도 지원한다
	UFUNCTION(Server, Reliable)
	void Server_MoveFromInvToInvRPC(class UInvObject* InvObj, class UInventoryComponent* TargetInv, int32 TargetTopLeftX, int32 TargetTopLeftY, bool bAutoFindSpace = true);
#pragma endregion

#pragma region /** Gameplay */
protected:
	// 인벤토리 오브젝트가 추가 혹은 제거 되었을 때 해당 오브젝트에 대한 로직을 처리하기 위해 호출된다
	virtual void Server_OnInvObjectAdded(class UInvObject* AddedObj);
	virtual void Server_OnInvObjectRemoved(class UInvObject* RemovedObj);
#pragma endregion

#pragma region /** Actions */
public:
	/// 모든 Try 함수들은 클라이언트 기준 Readonly여야 하며 Write는 서버 RPC를 통해서만 이루어져야 한다
	/// Try 함수의 반환 값은 서버에 요청을 보냈는지 여부이며, 실제로 그 요청이 처리되었는지를 반환하는 것은 아니다
	/// 실행 대상이 클라이언트일때, 하나의 Try 함수 내에서 서버에게 RPC 요청을 여러 번 보내는 것은 권장되지 않으며,
	/// 각 RPC 호출이 서로 어떠한 영향도 주지 않는 경우에만 허용된다.
	/// Try 함수 내에서 진행한 검증은 실제 데이터와 동일하다는 보장이 없기 때문에 서버에서 다시 한번 검증 후에 처리되어야 한다
	/// 그럼에도 클라단 검증을 진행하는 이유는 클라이언트 화면에서 아직 레플리케이션이 발생하지 않았는데 작업이 처리되는 것은 부자연스럽기 때문임

	// InvObject를 인벤토리에 추가하는 것을 시도하고 명령 요청 여부를 반환한다
	// NOTE: 이 함수는 아이템 액터의 삭제에는 관여하지 않는다, 아이템 액터를 픽업하기 위해서는 BaseItem에 구현된 함수를 사용할 것.
	UFUNCTION(BlueprintCallable)
	bool TryAddInvObject(class UInvObject* InvObj);

	// InvObject를 인벤토리의 주어진 격자 위치에 추가하는 것을 시도하고 명령 요청 여부를 반환한다
	UFUNCTION(BlueprintCallable)
	bool TryAddInvObjectAt(class UInvObject* InvObj, int32 GridX, int32 GridY);

	// InvObject를 인벤토리에서 제거하는 것을 시도하고 명령 요청 여부를 반환한다
	// NOTE: 이 함수는 아이템 액터를 생성하는 것에는 관여하지 않는다, 아이템 액터를 드랍하기 위해서는 TryDropInvObject를 사용할 것.
	UFUNCTION(BlueprintCallable)
	bool TryRemoveInvObject(class UInvObject* InvObj);

	// InvObject를 인벤토리에서 제거한 후, 이 컴포넌트의 부모의 드랍 위치에 아이템을 생성하고 아이템 데이터를 연동한다
	// AGameCharacter의 자식 클래스들의 경우 Muzzle 위치에 드랍한다
	UFUNCTION(BlueprintCallable)
	bool TryDropInvObject(class UInvObject* InvObj);

	// InvObject를 인벤토리에서 제거한 후, 입력으로 주어진 액터의 위치에 대응되는 아이템을 생성하고 아이템 데이터를 연동한다
	UFUNCTION(BlueprintCallable)
	bool TryDropInvObjectAtActorLocation(class UInvObject* InvObj, AActor* Actor);

	// InvObject를 인벤토리에서 제거한 후, 주어진 Location에 대응되는 아이템을 생성하고 아이템 데이터를 연동한다
	UFUNCTION(BlueprintCallable)
	bool TryDropInvObjectAt(class UInvObject* InvObj, FVector Location);

	// 소유한 모든 InvObject에 대해 드랍 처리를 한 번에 시도한다
	void Server_DropAllInvObjectImmediate(FVector Location, float RandomOffset);

	// 소유한 모든 InvObject에 대해 드랍 큐에 추가한다 
	void Server_DropAllInvObjectDeferred(FVector Location, float RandomOffset);

	// 로컬 값을 기준으로 인벤토리에 주어진 아이템이 들어갈 공간이 있는지 여부를 반환한다
	// 입력으로 주어진 X, Y 좌표가 아이템의 좌상단 위치라고 가정하고 판정한다
	// bAllowSelfCollision이 true일 경우, 인벤토리에 이미 해당 오브젝트가 들어있을 경우 자기 자신과의 충돌은 무시한다
	UFUNCTION(BlueprintCallable)
	bool Local_IsRoomAvailable(class UInvObject* InvObj, int32 CheckTopLeftX, int32 CheckTopLeftY, bool bAllowSelfCollision = false) const;

	// 인벤토리에 주어진 아이템이 들어갈 공간이 하나라도 존재하는지 여부를 반환한다
	UFUNCTION(BlueprintCallable)
	bool Local_HasSpaceFor(class UInvObject* InvObj, bool bAllowSelfCollision = false) const;

	// SrcX, SrcY에 위치한 아이템을 타깃 인벤토리의 TargetX, TargetY로 이동한다
	// 호출 대상과 타깃이 같을 경우에도 처리가 가능하다
	UFUNCTION(BlueprintCallable)
	bool TryMoveFromInvToInv(class UInvObject* InvObj, UInventoryComponent* TargetInv, int32 TargetX, int32 TargetY, bool bAutoFindSpace = false);
#pragma endregion

#pragma region /** Logic */
/* Replicated Data */
protected:
	// Key, Value를 같은 인덱스 순서로 짝지어 배열에 저장
	// TMap을 사용하지 않는 이유는 Replication이 지원되지 않기 때문

	// ObjId 목록 (Key)
	UPROPERTY(Replicated)
	TArray<int32> InvObjectKeys;

	// InvObject 목록 (Value)
	UPROPERTY(Replicated)
	TArray<class UInvObject*> InvObjectValues;

	// InvObject들의 좌상단 위치를 기록 (Value)
	// 필요 시 구조체 안에 데이터 추가 가능
	// TODO: Should Replicate
	UPROPERTY(Replicated)
	TArray<FInvObjData> InvObjectDatas;

public:
	// 주어진 InvObject의 id 반환
	// 찾을 수 없을 경우 INV_EMPTY 반환
	int32 GetObjId(class UInvObject* InvObj);

	// 주어진 id의 InvObject 반환
	UFUNCTION(BlueprintCallable)
	UInvObject* GetInvObjOfId(int32 ObjId) const;

	// 주어진 id의 ObjData 반환
	FInvObjData GetInvObjDataOfId(int32 ObjId) const;

	// 주어진 id key와 그에 대응되는 Value들을 삭제
	bool RemoveObjIdGroup(int32 ObjId);

	// 주어진 id key-Value들을 추가
	bool AddObjIdGroup(int32 ObjId, UInvObject* InvObj, FInvObjData ObjData);

	// 주어진 오브젝트를 다른 위치 혹은 다른 컴포넌트로 이동하는 것이 허용되는지 여부를 반환한다
	virtual bool CanMoveInvObj(UInvObject* InvObj);

	// 이 컴포넌트에서 주어진 오브젝트를 추가하는 게 허용되는지 여부를 반환한다
	virtual bool Server_CanAcceptInvObj(UInvObject* InvObj);

	// 소유중인 모든 InvObject를 어레이 형태로 반환
	TArray<UInvObject*> GetAllInvObjects() const;

	// 주어진 인덱스가 격자 배열의 유효 범위인지 확인
	bool IsValidGridIndex(int32 Index) const;
	
protected:
	// 주어진 크기에 맞는 새 격자를 생성한다
	// 하위 컴포넌트의 특성에 따라 필요 시 오버라이드해 수정이 가능하다
	void InitEmptyGrid();

	// 하위 컴포넌트에서 격자 크기에 대한 추가 로직이 필요할 경우 사용한다
	virtual void InitGridSize();

	// 주어진 격자 위치에 추가가 가능함이 검증된 InvObject를 인벤토리에 추가한다
	bool AddInvObject(class UInvObject* InvObj, int32 TopLeftX, int32 TopLeftY, int32 ObjId);

	// 인자로 주어진 id를 가지는 InvObject를 인벤토리에서 제거한다
	bool RemoveInvObject(int32 ObjId);

	// 주어진 id를 가지는 InvObject를 다른 인벤토리의 주어진 위치로 이동한다
	bool MoveFromInvToInv(int32 SelfObjId, UInventoryComponent* TargetInv, int32 TargetX, int32 TargetY);

	// 인벤토리에 값이 추가/삭제 되었을 경우 호출된다
	virtual void Server_OnInventoryContentChanged();

/* Object Id */
private:
	// 고유 id 생성을 위한 값으로, 마지막으로 발급된 고유 id 값을 기록
	UPROPERTY()
	int32 Server_LastObjId = INV_EMPTY + 1;

public:
	// 새 Obj ID를 발급 받는다
	FORCEINLINE int32 Server_IssueNewObjId();

/* Utility */
public:
	// 격자 상의 X, Y 좌표를 1차원 배열 인덱스로 변환
	FORCEINLINE int32 ConvertXYToArrayIndex(int32 X, int32 Y) const;

	// 1차원 배열 인덱스를 격자 상의 X, Y 좌표로 변환
	FORCEINLINE TTuple<int32, int32> ConvertArrayIndexToXY(int32 Index) const;

/* Grid */
protected:
	// 인벤토리 열
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid")
	int Columns = 8;

	// 인벤토리 행
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid")
	int Rows = 6;

	// 인벤토리 격자
	UPROPERTY(Replicated)
	TArray<int32> Grid;

public:
	// 인벤토리에 저장된 Obj들의 위치를 반환한다
	UFUNCTION(BlueprintCallable)
	const TMap<int32, FInvObjData> GetObjData() const;

	// 격자의 X, Y 좌표에 위치한 InvObject 반환
	UFUNCTION(BlueprintCallable)
	UInvObject* GetInvObjAtXY(int32 X, int32 Y) const;

	// 주어진 InvObject가 이 컴포넌트의 격자에서 실제로 차지하는 크기를 반환한다
	// 기본적으로 InvObject InvXSize, InvYSize를 사용한다
	// 오브젝트 크기에 영향을 받지 않는 인벤토리 등을 만들기 위해 이 컴포넌트의 자식 컴포넌트에서 이 함수를 수정해 사용할 수 있다
	virtual struct FInvObjSize GetGridDimensions(class UInvObject* InvObj) const;
#pragma endregion

#pragma region /** Interface */
protected:
	// 단일 타일 크기
	UPROPERTY(BlueprintReadWrite, Category = "Grid")
	float TileSize = INV_GRID_PIXEL;

	// 다음번 인벤토리 UI 호출 시 UI 세부 사항들을 업데이트 해야하는지 여부를 판정하기 위해 사용하는 값
	// RefreshCount와 값이 같아질 때까지 LocalRefreshCount에 1을 더하며 업데이트한다
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Grid")
	int32 RefreshCount = 0;

	// 서버, 클라 모두 사용하며 각 호스트마다 값이 다를 수 있다
	int32 LocalRefreshCount = 0;

public:
	// 이 호스트에 대해 UI의 강제 리프레시를 시도한다
	// 드래그 작업 실패 등 실 데이터는 변하지 않았으나 화면 상의 구성요소가 변했을 때 사용된다
	UFUNCTION(BlueprintCallable)
	void Local_ForceRefreshUI();

	// 현재 업데이트를 해야 하는지 여부를 반환한다
	UFUNCTION(BlueprintCallable)
	bool ShouldRefreshUI();

	// Refresh 할 경우 BP에서 호출한다
	UFUNCTION(BlueprintCallable)
	void OnRefreshUI();

	// 서버의 값이 변해 클라이언트들에게 Refresh를 해야함을 알리기 위해 사용한다
	// RefreshCount를 1 증가시킨다
	FORCEINLINE void Server_RegisterRefreshUI();
#pragma endregion

#pragma region /** Debug */
public:
	void PrintGrid() const;
#pragma endregion
};
