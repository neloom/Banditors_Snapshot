// Copyright (C) 2024 by Haguk Kim


#include "TRShop.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "ExpComponent.h"
#include "TRUtils.h"
#include "ProjectTRGameModeBase.h"

// Sets default values
ATRShop::ATRShop()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	InvComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("ShopInvComp"));
	check(InvComponent);

}

void ATRShop::BeginPlay()
{
	Super::BeginPlay();

	// 보유 아이템 개수가 많을 경우 눈치채기 어려운 정도의 일시적 프레임 드랍이 발생할 수 있다
	InitializeShop();
}

void ATRShop::Initialize()
{
	Super::Initialize();
}

void ATRShop::OnTriggered()
{
	TR_PRINT("ATRShop::OnTriggered()");
}

void ATRShop::InitializeShop()
{
	if (!HasAuthority())
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ATRShop::InitializeShop - Unable to get world!"));
		return;
	}
	AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
	if (!TRGM)
	{
		UE_LOG(LogTemp, Error, TEXT("ATRShop::InitializeShop - Unable to get game mode!"));
		return;
	}

	// 판매목록 자동 생성 필요 시 처리
	if (bAutoGenProducts)
	{
		Server_AutoInitProducts();
	}

	// 판매 아이템 초기화
	if (InvComponent)
	{
		TArray<FDropItem> ShopItems = TRUtils::SelectDropItems(this->SoldProducts);
		for (const FDropItem& Item : ShopItems)
		{
			if (Item.ItemRef)
			{
				ABaseItem* DroppedItem = TRGM->SpawnItem(Item.ItemRef, World, GetActorLocation(), FRotator(), FActorSpawnParameters());
				if (DroppedItem)
				{
					UInvObject* ItemInvObj = DroppedItem->GetInvObject();
					if (ItemInvObj)
					{
						// 판매 가능할 경우 픽업을 시도한다
						bool bAddResult = ItemInvObj->Local_IsMarketable() && DroppedItem->OnItemPickup(InvComponent);
						if (bAddResult)
						{
							// 추가 성공
							continue;
						}
					}

					UE_LOG(LogTemp, Error, TEXT("ATRShop::InitializeShop - Failed to add item %s to shop inventory! Item might be non-marketable, or the shop inventory might be full. This is considered normal."), *(DroppedItem->GetName()));
					DroppedItem->Destroy();
				}
			}
		}
	}
}

bool ATRShop::Local_CanPurchaseFromShop(AFPSCharacter* Buyer, UInvObject* CheckedItem)
{
	if (Buyer && Buyer->InvComponent && CheckedItem && CheckedItem->Local_IsMarketable() && Buyer->ExpComp && Buyer->ExpComp->GetCurrShard() >= CheckedItem->Local_GetPrice() && InvComponent && InvComponent->GetObjId(CheckedItem) != INV_EMPTY)
	{
		return true;
	}
	return false;
}

bool ATRShop::Local_CanSellToShop(AFPSCharacter* Seller, UInvObject* CheckedItem)
{
	// 현재 상점의 보유 재화량은 무한대이다
	if (Seller && Seller->InvComponent && CheckedItem && CheckedItem->Local_IsMarketable() && Seller->ExpComp && InvComponent && InvComponent->Local_HasSpaceFor(CheckedItem) && Seller->InvComponent->GetObjId(CheckedItem) != INV_EMPTY)
	{
		return true;
	}
	return false;
}

bool ATRShop::TryPurchaseFromShop(AFPSCharacter* Buyer, UInvObject* TradeItem)
{
	if (Local_CanPurchaseFromShop(Buyer, TradeItem))
	{
		Buyer->Server_RegisterShopPurchase(this, TradeItem);
		return true;
	}
	return false;
}

bool ATRShop::TrySellToShop(AFPSCharacter* Seller, UInvObject* TradeItem)
{
	if (Local_CanSellToShop(Seller, TradeItem))
	{
		Seller->Server_RegisterShopSell(this, TradeItem);
		return true;
	}
	return false;
}

void ATRShop::Server_PurchaseFromShop(AFPSCharacter* Buyer, UInvObject* OutItem)
{
	if (!HasAuthority()) return;
	if (!Buyer || !Buyer->InvComponent) return;

	// 구매자는 인벤토리 공간이 부족해도 살 수 있다
	if (!OutItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_PurchaseFromShop_Implementation - Invalid Object!"));
		return;
	}

	if (InvComponent && Local_CanPurchaseFromShop(Buyer, OutItem))
	{
		if (Server_TransferItem(OutItem, InvComponent, Buyer->InvComponent))
		{
			Buyer->ExpComp->SpendShard(OutItem->Local_GetPrice());
			return;
		}
	}
	return;
}

void ATRShop::Server_SellToShop(AFPSCharacter* Seller, UInvObject* InItem)
{
	if (!HasAuthority()) return;
	if (!Seller || !Seller->InvComponent) return;

	// 상점 인벤토리 공간이 부족할 경우 판매할 수 없다
	if (!InItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_SellToShop_Implementation - Invalid Object!"));
		return;
	}

	if (InvComponent && Local_CanSellToShop(Seller, InItem))
	{
		if (Server_TransferItem(InItem, Seller->InvComponent, InvComponent))
		{
			Seller->ExpComp->GainShard(InItem->Local_GetPrice());
			return;
		}
	}
	return;
}

bool ATRShop::Server_TransferItem(UInvObject* InvObj, UInventoryComponent* Origin, UInventoryComponent* Target)
{
	if (!HasAuthority()) return false;

	bool bTransactionResult = false;

	// 공간이 존재할 경우 InvObject를 이동한다
	if (Target->Local_HasSpaceFor(InvObj))
	{
		bTransactionResult = Origin->TryMoveFromInvToInv(InvObj, Target, 0, 0, true);
		if (!bTransactionResult)
		{
			UE_LOG(LogTemp, Error, TEXT("Server_TransferItem - Unexpected error! Target InvComp has space but the moving transaction has failed due to unknown reason!"));
		}
		return bTransactionResult;
	}

	// 공간이 부족한 경우 구매자 위치에 아이템을 드랍한다
	if (Target->GetOwner())
	{
		bTransactionResult = Origin->TryDropInvObjectAtActorLocation(InvObj, Target->GetOwner());
		if (!bTransactionResult)
		{
			UE_LOG(LogTemp, Error, TEXT("Server_TransferItem - Unexpected error! Dropping the item onto the buyer location has failed due to unknown reason!"));
		}
		return bTransactionResult;
	}

	UE_LOG(LogTemp, Error, TEXT("Server_TransferItem - Unexpected error! Please check immediately!"));
	return false;
}

void ATRShop::Server_AutoInitProducts()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World) return;
	AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
	if (TRGM)
	{
		int32 DungeonDepth = TRGM->GetCurrentDungeonDepth();
		int32 MaxRewardCount = Server_AutoSelectMaxProductCount(DungeonDepth);

		// 후보들을 다시 동적으로 선택한다
		SoldProducts.Empty();
		SoldProducts = TRUtils::FilterAndSelectCandidates(AutoGenSoldCandidates, DungeonDepth, MaxRewardCount);
	}
}

int32 ATRShop::Server_AutoSelectMaxProductCount(int32 DungeonDepth) const
{
	// TODO FIXME
	return 16;
}

