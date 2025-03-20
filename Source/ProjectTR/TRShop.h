// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonActor.h"
#include "InventoryComponent.h"
#include "TRStructs.h"
#include "Net/UnrealNetwork.h"
#include "TRShop.generated.h"

/**
* 상점 로직을 관리하는 액터로, 일반적인 상황에서는 이 액터를 직접 소환할 일은 발생하지 않는다
* InteractableShop 참고
*/
UCLASS()
class PROJECTTR_API ATRShop : public ADungeonActor
{
	GENERATED_BODY()
	
#pragma region /** Networking */
protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ATRShop, InvComponent);
	}
#pragma endregion

public:
	ATRShop();

protected:
	virtual void BeginPlay() override;
	virtual void Initialize() override;
	virtual void OnTriggered() override;

	// 최초 상점 초기화
	// 가판대에 놓을 아이템을 랜덤하게 선택한다
	void InitializeShop();

public:
	// 구매자가 해당 아이템을 판매/구매할 수 있는지 여부를 반환
	// 클라이언트에서도 호출이 가능하지만, 레플리케이션 속도에 따라 잘못된 값을 반환할 수 있다
	UFUNCTION(BlueprintCallable)
	bool Local_CanPurchaseFromShop(class AFPSCharacter* Buyer, class UInvObject* CheckedItem);

	UFUNCTION(BlueprintCallable)
	bool Local_CanSellToShop(class AFPSCharacter* Seller, class UInvObject* CheckedItem);

	// 로컬 트랜잭션 레지스터
	// 세부적인 함수 충족 기준은 InventoryComponent의 Try계열 함수들 주석을 참고할 것
	// 반환값은 호스트 단에서의 성공여부로, 서버에서도 성공하리라는 보장은 없음
	UFUNCTION(BlueprintCallable)
	bool TryPurchaseFromShop(class AFPSCharacter* Buyer, class UInvObject* TradeItem);

	UFUNCTION(BlueprintCallable)
	bool TrySellToShop(class AFPSCharacter* Seller, class UInvObject* TradeItem);

	// 구매 트랜잭션 처리
	// 수행 결과를 반환한다
	// NOTE: 이 함수를 처리하기 전에 반드시 주어진 ObjId가 적합한 InvComponent에 있는지 확인해야 하며, (여기서는 상점 인벤토리)
	// 확인 과정에서 다른 InvComponent를 사용하지 않도록 주의해야 한다
	void Server_PurchaseFromShop(class AFPSCharacter* Buyer, class UInvObject* OutItem);

	// 판매 트랜잭션 처리
	// 수행 결과를 반환한다
	// NOTE: 이 함수를 처리하기 전에 반드시 주어진 ObjId가 적합한 InvComponent에 있는지 확인해야 하며, (여기서는 판매자의 인벤토리)
	// 확인 과정에서 다른 InvComponent를 사용하지 않도록 주의해야 한다
	void Server_SellToShop(class AFPSCharacter* Seller, class UInvObject* InItem);
	
protected:
	// 아이템 이동
	bool Server_TransferItem(class UInvObject* InvObj, UInventoryComponent* Origin, UInventoryComponent* Target);

	// 층에 따른 판매 목록 랜덤 생성
	void Server_AutoInitProducts();

	// 상점 판매 아이템 후보의 개수 지정
	// NOTE: 실제 판매 아이템 개수가 아닌, 후보의 개수이므로 실제 판매수량은 이 값 이하가 된다
	int32 Server_AutoSelectMaxProductCount(int32 DungeonDepth) const;

public:
	// 아이템 관리를 위한 인벤토리 컴포넌트
	UPROPERTY(Replicated, BlueprintReadOnly)
	UInventoryComponent* InvComponent = nullptr;

public:
	// 상점에서 판매 가능한 아이템의 품목들
	// 최초 초기화 이후 이 값은 사용되지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FDropItem> SoldProducts;

	// 이 값이 참일 경우 클래스 디폴트에 등록된 SoldProducts 정보를 무시하고 
	// 대신 이 인스턴스가 속한 콘텍스트에 맞는 리워드를 자동으로 랜덤하게 선택해 배정한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	bool bAutoGenProducts = false;

	// 판매 목록 항목 자동 생성 시 사용할 후보들
	// 만약 bAutoGenProducts이 false라면 이 값은 상점 판매목록에 아무런 영향을 주지 않는다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	TArray<FUnfilteredDropItem> AutoGenSoldCandidates;
};
