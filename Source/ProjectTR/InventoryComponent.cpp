// Copyright (C) 2024 by Haguk Kim


#include "InventoryComponent.h"
#include "InvObject.h"
#include "ItemData.h"
#include "BaseItem.h"
#include "EquipSystem.h"
#include "ProjectTRGameModeBase.h"
#include "GameCharacter.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// 레플리케이션 사용
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	// 데이터 초기화
	Initialize();
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::Initialize()
{
	InvObjectKeys.Empty();
	InvObjectValues.Empty();
	InvObjectDatas.Empty();

	InitGridSize();
	InitEmptyGrid();
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	////// DEBUG
	/*FColor c = FColor::Blue;
	if (GetOwner()->HasAuthority()) c = FColor::Red;
	FString s1 = FString::Printf(TEXT("%s"), *(GetOwner()->GetName()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, c, s1);
	FString s2;
	for (int i = 0; i < 4; ++i)
	{
		TTuple<int32, int32> tt = ConvertArrayIndexToXY(i);
		UInvObject* io = GetInvObjAtXY(tt.Get<0>(), tt.Get<1>());
		if (!io)
		{
			s2 += FString::Printf(TEXT("nullptr "));
		}
		else
		{
			s2 += FString::Printf(TEXT("%s "), *io->GetName());
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, c, s2);
	FString s3;
	for (int i = 0; i < 4; ++i)
	{
		s3 += FString::Printf(TEXT("%d "), Grid[i]);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, c, s3);
	FString s4;
	for (const int32& k : InvObjectKeys)
	{
		s4 += FString::Printf(TEXT("%d "), k);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, c, s4);
	FString s5 = FString::Printf(TEXT("InvObjValuesNum: %d"), InvObjectValues.Num());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, c, s5);
	FString s6;
	for (const FInvObjData& d : InvObjectDatas)
	{
		s6 += FString::Printf(TEXT("XY:%d %d "), d.GridX, d.GridY);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, c, s6);*/
}

void UInventoryComponent::Server_ExportInventoryInfo(TArray<class UInvObject*>* OutValues, TArray<FInvObjData>* OutDatas)
{
	check(InvObjectValues.Num() == InvObjectDatas.Num());

	TArray<UInvObject*> NewValues = TArray<UInvObject*>();
	TArray<FInvObjData> NewDatas = TArray<FInvObjData>();

	for (int32 Index = 0; Index < InvObjectKeys.Num(); ++Index)
	{
		UInvObject* CValue = nullptr;
		if (InvObjectValues[Index])
		{
			CValue = InvObjectValues[Index];

			// GC 해제(루트셋 추가) 및 아우터 변경
			CValue->ChangeRootSetRecursive(true, GetWorld()->GetAuthGameMode()->GetGameInstance());
		}
		FInvObjData CData = FInvObjData(InvObjectDatas[Index]);

		NewValues.Add(CValue);
		NewDatas.Add(CData);
	}

	*OutValues = NewValues;
	*OutDatas = NewDatas;
}

void UInventoryComponent::Server_ImportInvComp(TArray<class UInvObject*> NewValues, TArray<FInvObjData> NewDatas)
{
	check(NewValues.Num() == NewDatas.Num());

	// 임포트 시작
	bIsImportingInventory = true;

	// 오브젝트들을 루트셋으로부터 제거한다
	// 이는 메모리 리킹을 방지하기 위함임
	// 이 과정은 임포트의 최상단부에서 진행하는 게 권장된다; 루트셋 리킹 발생으로 크래시가 날 수 있기 때문
	for (UInvObject* DeletePendingObj : NewValues)
	{
		DeletePendingObj->ChangeRootSetRecursive(false, GetOwner());
	}

	// 컴포넌트 필수 초기화가 진행되어 있지 않은 경우 초기화를 진행한다
	// 초기화가 이루어져야 인벤토리 격자 크기가 할당되기 때문에 필수적이다
	Initialize();

	// 인벤토리의 각 오브젝트를 생성해 기존 위치대로 재배치한다
	// InvObject는 아이템의 생성에 의해 부차적으로 생성해야 하므로
	// 임의의 위치에 아이템을 생성하고 곧바로 인벤토리에 추가 후 위치를 변경하는 형태로 복구한다
	// 클라이언트에게는 Spawn 과정에서 레플리케이션이 적용된다
	for (int32 Index = 0; Index < NewValues.Num(); ++Index)
	{
		UInvObject* InvObj = NewValues[Index];
		if (InvObj && InvObj->GetBaseItemClass() && InvObj->GetItemData())
		{
			AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(GetWorld()->GetAuthGameMode());
			if (TRGM)
			{
				ABaseItem* TempItem = TRGM->RespawnItem(InvObj->GetBaseItemClass(), GetWorld(), FVector(), FRotator(), FActorSpawnParameters(), InvObj, InvObj->GetItemData());
				if (TempItem && TempItem->GetInvObject())
				{
					// 레벨 트랜지션 이후에는 기존 아이콘 생성 여부와 무관하게 아이콘을 재생성한다
					TempItem->Server_InitializeIcon();

					UInvObject* NewObj = TempItem->GetInvObject();
					if (!TempItem->OnItemPickup(this))
					{
						UE_LOG(LogTemp, Error, TEXT("Server_ImportInvComp - Something went wrong during inventory restoring!"));
					}
					Server_MoveFromInvToInvRPC(NewObj, this, NewDatas[Index].GridX, NewDatas[Index].GridY);

					// InvObject와 다르게 기존 ItemData는 그대로 재사용함
					// 이때 이미 데이터를 기반으로 액터 멤버들에 대한 복구는 완료된 상황이므로, 데이터만 바꿔주면 됨
					NewObj->SetItemData(InvObj->GetItemData());
					NewObj->GetItemData()->Rename(nullptr, this->GetOwner()); // NOTE: 아우터를 캐릭터로 설정 (OnItemPickup -> CacheBeforeDestruction 참고)
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Server_ImportInvComp - Something went wrong during the respawn!"));
					continue;
				}
			}
		}
	}

	// 임포트 종료
	bIsImportingInventory = false;
}

void UInventoryComponent::Server_SpawnItemFromInvObjectRPC_Implementation(UInvObject* InvObj, FVector Location)
{
	ABaseItem* GeneratedItem = InvObj->GenerateAndSpawnItem(GetWorld(), Location, FRotator(), FActorSpawnParameters());
	return;
}

void UInventoryComponent::Server_MoveFromInvToInvRPC_Implementation(UInvObject* InvObj, UInventoryComponent* TargetInv, int32 TargetTopLeftX, int32 TargetTopLeftY, bool bAutoFindSpace)
{
	if (!GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_MoveFromInvToInvRPC - InventoryComponent has no owner! It must have a default owner set to the server."));
		return;
	}
	if (!IsValid(InvObj))
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_MoveFromInvToInvRPC - InvObj is not valid."));
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		int32 ObjId = GetObjId(InvObj);
		if (ObjId == INV_EMPTY)
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot move an object that is not in the inventory component!"));
			// 클라에서 싱크가 맞지 않아 잘못된 요청이 왔을 가능성이 있으므로 강제 보정 시도
			Server_RegisterRefreshUI();
			return;
		}

		if (TargetInv->Local_IsRoomAvailable(InvObj, TargetTopLeftX, TargetTopLeftY, true))
		{
			if (MoveFromInvToInv(ObjId, TargetInv, TargetTopLeftX, TargetTopLeftY))
			{
				// Replication의 주체를 TargetInv로 옮긴다 (TargetInv는 this일 수도 있다)
				this->RemoveReplicatedSubObject(InvObj);
				TargetInv->AddReplicatedSubObject(InvObj);
				return;
			}
		}
		else if (bAutoFindSpace)
		{
			for (int32 gridY = 0; gridY < Rows; ++gridY)
			{
				for (int32 gridX = 0; gridX < Columns; ++gridX)
				{
					if (TargetInv->Local_IsRoomAvailable(InvObj, gridX, gridY, true))
					{
						if (MoveFromInvToInv(ObjId, TargetInv, gridX, gridY))
						{
							this->RemoveReplicatedSubObject(InvObj);
							TargetInv->AddReplicatedSubObject(InvObj);
							return;
						}
					}
				}
			}
		}
	}
}

void UInventoryComponent::Server_OnInvObjectAdded(UInvObject* AddedObj)
{
	if (!AddedObj) return;

	// 컴포넌트 오너가 게임캐릭터일 경우의 로직
	AGameCharacter* CharOwner = Cast<AGameCharacter>(GetOwner());
	if (CharOwner && CharOwner->HasAuthority())
	{
		// 스테이터스 이펙트
		if (AddedObj->bApplyStatusEffectToOwner)
		{
			UStatusEffect* OwnerStatusEffect = CharOwner->Server_GenerateAndAddStatEffect(AddedObj->OwnerStatusEffectData, CharOwner/* 스스로 픽업함 */);
			if (OwnerStatusEffect)
			{
				AddedObj->CachedOwnerStatEffect = MakeWeakObjectPtr<UStatusEffect>(OwnerStatusEffect);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Server_OnInvObjectAdded - New UStatusEffect instance was not generated. This could happen in certain scenarios(e.g. item grants temporary stateff after pickup), but most likely something has gone wrong."));
			}
		}
	}
}

void UInventoryComponent::Server_OnInvObjectRemoved(UInvObject* RemovedObj)
{
	if (!RemovedObj) return;

	// 컴포넌트 오너가 게임캐릭터일 경우의 로직
	AGameCharacter* CharOwner = Cast<AGameCharacter>(GetOwner());
	if (CharOwner && CharOwner->HasAuthority())
	{
		CharOwner->Server_RemoveStatEffect(RemovedObj->CachedOwnerStatEffect.Get());
	}
}

bool UInventoryComponent::TryAddInvObject(UInvObject* InvObj)
{
	Local_ForceRefreshUI();
	if (IsValid(InvObj))
	{
		for (int32 gridY = 0; gridY < Rows; ++gridY)
		{
			for (int32 gridX = 0; gridX < Columns; ++gridX)
			{
				if (Local_IsRoomAvailable(InvObj, gridX, gridY))
				{
					Server_AddInvObjectRPC(InvObj, gridX, gridY);
					return true;
				}
			}
		}
	}
	return false;
}

bool UInventoryComponent::TryAddInvObjectAt(UInvObject* InvObj, int32 GridX, int32 GridY)
{
	Local_ForceRefreshUI();
	if (IsValid(InvObj))
	{
		if (Local_IsRoomAvailable(InvObj, GridX, GridY))
		{
			Server_AddInvObjectRPC(InvObj, GridX, GridY);
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::TryRemoveInvObject(UInvObject* InvObj)
{
	Local_ForceRefreshUI();
	if (IsValid(InvObj) && CanMoveInvObj(InvObj))
	{
		for (int idx = 0; idx < InvObjectKeys.Num(); ++idx)
		{
			if (InvObjectValues[idx] == InvObj)
			{
				Server_RemoveInvObjectRPC(InvObjectKeys[idx]);
				return true;
			}
		}
	}
	return false;
}

bool UInventoryComponent::TryDropInvObject(UInvObject* InvObj)
{
	if (!GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("InvComp has no owner. Failed to drop InvObject at owner location."));
		return false;
	}
	if (AGameCharacter* GameCharacter = Cast<AGameCharacter>(GetOwner()))
	{
		return TryDropInvObjectAt(InvObj, GameCharacter->GetHandPointInfo().Get<0>());
	}
	return TryDropInvObjectAtActorLocation(InvObj, GetOwner());
}

bool UInventoryComponent::TryDropInvObjectAtActorLocation(UInvObject* InvObj, AActor* Actor)
{
	return TryDropInvObjectAt(InvObj, Actor->GetActorLocation());
}

bool UInventoryComponent::TryDropInvObjectAt(UInvObject* InvObj, FVector Location)
{
	Local_ForceRefreshUI();
	if (TryRemoveInvObject(InvObj))
	{
		Server_SpawnItemFromInvObjectRPC(InvObj, Location);
		return true;
	}
	return false;
}

void UInventoryComponent::Server_DropAllInvObjectImmediate(FVector Location, float RandomOffset)
{
	AGameCharacter* Owner = Cast<AGameCharacter>(GetOwner());
	if (!Owner || !Owner->HasAuthority()) return;
	TArray<int32> OriginInvObjId;
	for (int32 ObjId : InvObjectKeys)
	{
		OriginInvObjId.Emplace(ObjId);
	}
	for (int32 DropObjId : OriginInvObjId)
	{
		UInvObject* DropObj = GetInvObjOfId(DropObjId);

		FVector2D XYOffset = FMath::RandPointInCircle(RandomOffset);
		Server_SpawnItemFromInvObjectRPC(DropObj, FVector(Location.X + XYOffset.X, Location.Y + XYOffset.Y, Location.Z));

		RemoveInvObject(DropObjId);
	}
}

void UInventoryComponent::Server_DropAllInvObjectDeferred(FVector Location, float RandomOffset)
{
	AGameCharacter* Owner = Cast<AGameCharacter>(GetOwner());
	if (!Owner || !Owner->HasAuthority()) return;
	TArray<int32> OriginInvObjId;
	for (int32 ObjId : InvObjectKeys)
	{
		OriginInvObjId.Emplace(ObjId);
	}
	for (int32 DropObjId : OriginInvObjId)
	{
		UInvObject* DropObj = GetInvObjOfId(DropObjId);

		FVector2D XYOffset = FMath::RandPointInCircle(RandomOffset);
		Owner->Server_AddDeferredDropItem(DropObj, FVector(Location.X + XYOffset.X, Location.Y + XYOffset.Y, Location.Z));

		RemoveInvObject(DropObjId); // 인벤토리에서는 드랍 사전에 제거한다
	}
}

bool UInventoryComponent::Local_IsRoomAvailable(UInvObject* InvObj, int32 CheckTopLeftX, int32 CheckTopLeftY, bool bAllowSelfCollision) const
{
	if (!InvObj) return false;

	FInvObjSize ObjSize = GetGridDimensions(InvObj);
	if (ObjSize.X <= 0 || ObjSize.Y <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("InvObject %s has invalid size."), *InvObj->GetName());
		return false;
	}
	if (CheckTopLeftX + ObjSize.X > Columns || CheckTopLeftY + ObjSize.Y > Rows || CheckTopLeftX < 0 || CheckTopLeftY < 0)
	{
		return false;
	}

	for (int32 y = CheckTopLeftY; y < CheckTopLeftY + ObjSize.Y; ++y)
	{
		for (int32 x = CheckTopLeftX; x < CheckTopLeftX + ObjSize.X; ++x)
		{
			int32 ArrayIndex = ConvertXYToArrayIndex(x, y);
			if (!IsValidGridIndex(ArrayIndex))
			{
				return false;
			}
			if (Grid[ArrayIndex] != INV_EMPTY)
			{
				if (!bAllowSelfCollision) return false;
				else if (GetInvObjOfId(Grid[ArrayIndex]) != InvObj) return false;
			}
		}
	}
	return true;
}

bool UInventoryComponent::Local_HasSpaceFor(UInvObject* InvObj, bool bAllowSelfCollision) const
{
	for (int32 gridY = 0; gridY < Rows; ++gridY)
	{
		for (int32 gridX = 0; gridX < Columns; ++gridX)
		{
			if (Local_IsRoomAvailable(InvObj, gridX, gridY, bAllowSelfCollision))
			{
				return true;
			}
		}
	}
	return false;
}

bool UInventoryComponent::TryMoveFromInvToInv(class UInvObject* InvObj, UInventoryComponent* TargetInv, int32 TargetX, int32 TargetY, bool bAutoFindSpace)
{
	Local_ForceRefreshUI();
	TargetInv->Local_ForceRefreshUI();
	if (IsValid(TargetInv))
	{
		if (IsValid(InvObj) && CanMoveInvObj(InvObj))
		{
			if (TargetInv->Local_IsRoomAvailable(InvObj, TargetX, TargetY, true))
			{
				Server_MoveFromInvToInvRPC(InvObj, TargetInv, TargetX, TargetY, bAutoFindSpace);
				return true;
			}
			else if (bAutoFindSpace)
			{
				for (int32 gridY = 0; gridY < Rows; ++gridY)
				{
					for (int32 gridX = 0; gridX < Columns; ++gridX)
					{
						if (TargetInv->Local_IsRoomAvailable(InvObj, gridX, gridY, true))
						{
							Server_MoveFromInvToInvRPC(InvObj, TargetInv, gridX, gridY, true);
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool UInventoryComponent::CanMoveInvObj(UInvObject* InvObj)
{
	if (!IsValid(InvObj)) return false;
	return true;
}

bool UInventoryComponent::Server_CanAcceptInvObj(UInvObject* InvObj)
{
	return true; // 기본적으로 전부 수용 가능
}

TArray<UInvObject*> UInventoryComponent::GetAllInvObjects() const
{
	return TArray<UInvObject*>(InvObjectValues); // 복사
}

bool UInventoryComponent::IsValidGridIndex(int32 Index) const
{
	if (Index < 0 || Index >= Rows * Columns) return false;
	return true;
}

int32 UInventoryComponent::Server_IssueNewObjId()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Error, TEXT("Non-authoritative actor cannot issue a new object id. If the actor is not a player, set its owner to server."));
		return 0;
	}
	Server_LastObjId = (Server_LastObjId + 1) % INT32_MAX;
	if (Server_LastObjId <= INV_EMPTY) Server_LastObjId = INV_EMPTY + 1;
	return Server_LastObjId;
}

int32 UInventoryComponent::ConvertXYToArrayIndex(int32 X, int32 Y) const
{
	return Y * (int32)Columns + X;
}

TTuple<int32, int32> UInventoryComponent::ConvertArrayIndexToXY(int32 Index) const
{
	return TTuple<int32, int32>(Index % (int32)Rows, (Index / (int32)Rows));
}

const TMap<int32, FInvObjData> UInventoryComponent::GetObjData() const
{
	check(InvObjectKeys.Num() == InvObjectValues.Num());
	check(InvObjectKeys.Num() == InvObjectDatas.Num());
	TMap<int32, FInvObjData> ObjMap;
	for (int idx = 0; idx < InvObjectKeys.Num(); ++idx)
	{
		ObjMap.Add({ InvObjectKeys[idx], InvObjectDatas[idx] });
	}
	return ObjMap;
}

UInvObject* UInventoryComponent::GetInvObjAtXY(int32 X, int32 Y) const
{
	if (X >= 0 && X < Columns && Y >= 0 && Y < Rows)
	{
		int32 Index = ConvertXYToArrayIndex(X, Y);
		if (IsValidGridIndex(Index) && IsValid(GetInvObjOfId(Grid[Index])))
		{
			return GetInvObjOfId(Grid[Index]);
		}
	}
	return nullptr;
}

FInvObjSize UInventoryComponent::GetGridDimensions(UInvObject* InvObj) const
{
	return InvObj->GetDimensions();
}

int32 UInventoryComponent::GetObjId(UInvObject* InvObj)
{
	int32 index = InvObjectValues.Find(InvObj);
	if (index != INDEX_NONE)
	{
		return InvObjectKeys[index];
	}
	return INV_EMPTY;
}

UInvObject* UInventoryComponent::GetInvObjOfId(int32 ObjId) const
{
	if (ObjId == INV_EMPTY) return nullptr;
	for (int idx = 0; idx < InvObjectKeys.Num(); ++idx)
	{
		if (InvObjectKeys[idx] == ObjId)
		{
			return InvObjectValues[idx];
		}
	}
	return nullptr;
}

void UInventoryComponent::InitGridSize()
{
	return; // overridable
}

void UInventoryComponent::InitEmptyGrid()
{
	Grid.Empty();
	for (int i = 0; i < Rows * Columns; ++i)
	{
		Grid.Add(INV_EMPTY);
	}
}

void UInventoryComponent::Local_ForceRefreshUI()
{
	LocalRefreshCount = RefreshCount - 1;
}

bool UInventoryComponent::ShouldRefreshUI()
{
	if (RefreshCount < LocalRefreshCount)
	{
		UE_LOG(LogTemp, Error, TEXT("Something went wrong. Local host should never refresh more than the server does. This could also happen if RefreshCount passes integer limit."));
		LocalRefreshCount = RefreshCount;
		return true; // Refresh
	}

	// 오차가 굉장히 큰 경우 값을 1만 차이나게 수정한다
	// 다른 호스트들에 의해 빈번하게 업데이트 되었을 경우 발생할 수 있다
	if (RefreshCount - LocalRefreshCount > MAX_REFR_DIFF)
	{
		LocalRefreshCount = RefreshCount - 1;
	}
	return RefreshCount > LocalRefreshCount;
}

void UInventoryComponent::OnRefreshUI()
{
	LocalRefreshCount = (LocalRefreshCount + 1) % INT32_MAX;
}

void UInventoryComponent::Server_RegisterRefreshUI()
{
	if (GetOwner()->HasAuthority()) RefreshCount = (RefreshCount + 1) % INT32_MAX;
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Clients should not call server-only functions."));
	}
}

void UInventoryComponent::PrintGrid() const
{
	for (int i = 0; i < Rows; ++i)
	{
		FString RowString = "";
		for (int j = 0; j < Columns; ++j)
		{
			RowString += FString::Printf(TEXT("%d "), Grid[ConvertXYToArrayIndex(j, i)]);
		}
		UE_LOG(LogTemp, Error, TEXT("%s"), *RowString);
	}
}

FInvObjData UInventoryComponent::GetInvObjDataOfId(int32 ObjId) const
{
	for (int idx = 0; idx < InvObjectKeys.Num(); ++idx)
	{
		if (InvObjectKeys[idx] == ObjId)
		{
			return InvObjectDatas[idx];
		}
	}

	checkf(false, TEXT("Unable to find appropriate FInvObjData"));
	return FInvObjData();
}

bool UInventoryComponent::RemoveObjIdGroup(int32 ObjId)
{
	for (int idx = 0; idx < InvObjectKeys.Num(); ++idx)
	{
		if (InvObjectKeys[idx] == ObjId)
		{
			InvObjectKeys.RemoveAt(idx);
			InvObjectValues.RemoveAt(idx);
			InvObjectDatas.RemoveAt(idx);
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::AddObjIdGroup(int32 ObjId, UInvObject* InvObj, FInvObjData ObjData)
{
	check(InvObjectKeys.Num() == InvObjectValues.Num());
	check(InvObjectKeys.Num() == InvObjectDatas.Num());
	InvObjectKeys.Add(ObjId);
	InvObjectValues.Add(InvObj);
	InvObjectDatas.Add(ObjData);
	return true;
}

bool UInventoryComponent::AddInvObject(UInvObject* InvObj, int32 TopLeftX, int32 TopLeftY, int32 ObjId)
{
	if (!IsValid(InvObj))
	{
		UE_LOG(LogTemp, Error, TEXT("Can't add invalid InvObject."));
		return false;
	}
	if (ObjId == INV_EMPTY)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't set ObjId equal to INV_EMPTY."));
		return false;
	}
	if (!Server_CanAcceptInvObj(InvObj)) return false;

	// 위치 정보 생성
	FInvObjData Data;
	Data.GridX = TopLeftX;
	Data.GridY = TopLeftY;

	// 아이템 정보 기록
	if (!AddObjIdGroup(ObjId, InvObj, Data))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to execute AddObjIdGroup!"));
		return false;
	};

	// Grid 채우기
	FInvObjSize GridSize = GetGridDimensions(InvObj);
	int32 XSize = GridSize.X;
	int32 YSize = GridSize.Y;
	for (int32 Y = TopLeftY; Y < TopLeftY + YSize; ++Y)
		for (int32 X = TopLeftX; X < TopLeftX + XSize; ++X)
			Grid[ConvertXYToArrayIndex(X, Y)] = ObjId;

	Server_RegisterRefreshUI();
	Server_OnInventoryContentChanged();
	Server_OnInvObjectAdded(InvObj);
	return true;
}

void UInventoryComponent::Server_AddInvObjectRPC_Implementation(UInvObject* InvObj, int32 TopLeftX, int32 TopLeftY)
{
	if (!GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent has no owner! It must have a default owner set to the server."));
		return;
	}
	if (GetOwner()->HasAuthority())
	{
		AddReplicatedSubObject(InvObj);
		int32 ObjId = Server_IssueNewObjId();
		AddInvObject(InvObj, TopLeftX, TopLeftY, ObjId);
	}
}

bool UInventoryComponent::RemoveInvObject(int32 ObjId)
{
	if (GetInvObjOfId(ObjId) == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjId is not valid! Removal has failed."));
		return false;
	}
	UInvObject* InvObj = GetInvObjOfId(ObjId);
	if (!IsValid(InvObj))
	{
		UE_LOG(LogTemp, Error, TEXT("InvObj is not valid! Removal has failed."));
		return false;
	}

	// 위치 정보 가져오기
	FInvObjData Data = GetInvObjDataOfId(ObjId);
	int32 TopLeftX = Data.GridX;
	int32 TopLeftY = Data.GridY;

	// 아이템 정보 제거
	if (!RemoveObjIdGroup(ObjId))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to execute RemoveObjIdGroup!"));
		return false;
	}

	// Grid 비우기
	FInvObjSize GridSize = GetGridDimensions(InvObj);
	int32 XSize = GridSize.X;
	int32 YSize = GridSize.Y;
	for (int32 Y = TopLeftY; Y < TopLeftY + YSize; ++Y)
		for (int32 X = TopLeftX; X < TopLeftX + XSize; ++X)
			Grid[ConvertXYToArrayIndex(X, Y)] = INV_EMPTY;

	Server_RegisterRefreshUI();
	Server_OnInventoryContentChanged();
	Server_OnInvObjectRemoved(InvObj);
	return true;
}

bool UInventoryComponent::MoveFromInvToInv(int32 SelfObjId, UInventoryComponent* TargetInv, int32 TargetX, int32 TargetY)
{
	UInvObject* InvObj = GetInvObjOfId(SelfObjId);
	if (!IsValid(InvObj)) return false;
	if (!TargetInv->Server_CanAcceptInvObj(InvObj)) return false; // 타깃의 수용 가능 여부를 미리 확인해야함

	if (RemoveInvObject(SelfObjId))
	{
		int32 ObjId = TargetInv->Server_IssueNewObjId();
		return TargetInv->AddInvObject(InvObj, TargetX, TargetY, ObjId);
	}
	return false;
}

void UInventoryComponent::Server_OnInventoryContentChanged()
{
	// 필요 시 오버라이드 후 로직 작성
}

void UInventoryComponent::Server_RemoveInvObjectRPC_Implementation(int32 ObjId)
{
	if (!GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent has no owner! It must have a default owner set to the server."));
		return;
	}
	if (GetOwner()->HasAuthority())
	{
		RemoveReplicatedSubObject(GetInvObjOfId(ObjId));
		RemoveInvObject(ObjId);
	}
}

