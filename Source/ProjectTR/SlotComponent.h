// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "InvObject.h"
#include "SlotComponent.generated.h"

/*
* 행 하나의 형태로 값을 저장한다
* 즉 x번 인덱스 슬롯은 그리드의 (x,0)에 저장된다
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTTR_API USlotComponent : public UInventoryComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USlotComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME(USlotComponent, CurrentSlot);
	}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 격자 크기는 슬롯 크기에 비례하게 설정
	virtual void InitGridSize() override;

	// 모든 InvObject들이 격자 1x1칸만 차지하도록 함수 오버라이딩
	virtual struct FInvObjSize GetGridDimensions(class UInvObject* InvObj) const override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 현재 선택된 슬롯으로 변경
	UFUNCTION(BlueprintCallable)
	void SwitchSlotTo(int32 SlotIdx);

	// 슬롯 번호가 주어졌을 때 InvObject 반환
	// 할당된 오브젝트가 없거나 슬롯 번호가 부정확할 경우 nullptr 반환
	UFUNCTION(BlueprintCallable)
	class UInvObject* GetInvObjOfSlot(int32 SlotIdx) const;

	// 현재 슬롯에 할당된 InvObject 반환
	UFUNCTION(BlueprintCallable)
	class UInvObject* GetInvObjOfCurrSlot() const;

	// InvObject를 해당 슬롯에 대응되는 격자에 추가
	UFUNCTION(BlueprintCallable)
	bool TryAddInvObjToSlot(class UInvObject* InvObj, int32 SlotIdx);

	// 현재 슬롯 번호를 반환한다
	int32 GetCurrentSlot() const { return CurrentSlot; }

	// 슬롯 개수를 반환한다
	const int32 GetSlotCount() const { return SlotCount; }

protected:
	// 현재 선택된 슬롯 번호
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 CurrentSlot = 0;

	// 슬롯 개수 (생성 후 고정)
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 SlotCount = 10;
};
