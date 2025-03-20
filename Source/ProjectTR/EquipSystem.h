// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameCharacter.h"
#include "BaseItem.h"
#include "SlotComponent.h"
#include "Net/UnrealNetwork.h"
#include "EquipSystem.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTTR_API UEquipSystem : public USlotComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipSystem();

	// 네트워크
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UEquipSystem, CurrWeaponActor);
	}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 이 컴포넌트의 소유 캐릭터를 반환한다
	TObjectPtr<AGameCharacter> GetTROwner();

	// 사용 준비중인 무기 및 무기 슬롯 번호를 변경한다
	// 즉 준비중인 무기를 기존 위치에 집어넣고 새 무기를 사용 준비 상태로 만든다
	bool Server_SwitchWeaponSlotTo(int32 SlotIdx);

	// 애니메이션을 미리 재생하기 위해 무기 액터가 아닌 WeaponState는 따로 RPC로 전체 클라이언트에 공유한다
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSwitchWeapon(EHumanoidWeaponState WeaponState);

	// 현재 사용 대기중인 무기 액터가 있을 경우 사용 대기 상태를 해제한다
	// 장착 해제와는 별개의 개념으로, InvObj가 슬롯에는 남아있으나 액터는 파괴된다
	void Server_RetrieveCurrWeapon();

	// 해당 슬롯에 아이템을 추가한다
	// 이미 해당 위치에 장착된 아이템이 있을 경우 아무 것도 수행하지 않는다
	bool TryEquipItemAt(UInvObject* InvObj, int32 SlotIdx);

	// 서버, 클라이언트 공통 로직
	// WeaponActor를 어태치/디태치한다
	// NOTE: 디태치의 경우, 이미 Destroy된 상태의 값을 Replicate 받는 것이기 때문에 실질적으로 클라이언트측에서는 디태치할 일이 없다
	void ProcessWeaponAttachment();
	void ProcessWeaponDetachment();

	// 서버, 클라이언트 공통 로직
	// Deploy된 무기가 존재할 경우 해당 무기의 Recoil을 초기화한다
	void ProcessWeaponRecoilInit();

	// 주어진 위치에 있는 아이템을 다른 위치 혹은 다른 컴포넌트로 이동하는 것이 허용되는지 여부를 반환한다
	// 현재 Deploy중인 경우 false를 반환한다
	virtual bool CanMoveInvObj(UInvObject* InvObj) override;

	// 이 컴포넌트에서 주어진 오브젝트를 추가하는 게 허용되는지 여부를 반환한다
	// Wield 가능한 경우에만 accept한다
	virtual bool Server_CanAcceptInvObj(UInvObject* InvObj);

	// Getter
	// 서버와 클라 모두에서 사용할 수 있지만, 클라의 경우 네트워크 딜레이로 인해 부정확한 값을 반환할 수 있다
	class AWieldItem* GetCurrWeaponActor();

protected:
	UFUNCTION()
	void OnRep_CurrWeaponActor();

	// Setter
	void Server_SetCurrWeaponActor(class AWieldItem* NewWeapon);

	// CurrWeaponActor가 변경된 이후 무기에 바인딩된 WielderStatusEffect의 갱신이 필요하다
	void Server_OnCurrWeaponActorChanged(class AWieldItem* OldWeapon);

	// EquipSystem의 경우 콘텐츠가 변경되었을 때 Deploy 상태도 그에 맞게 업데이트한다
	virtual void Server_OnInventoryContentChanged() override;

	// UI 갱신
	void Local_OnCurrWeaponActorUpdated();

protected:
	// 현재 사용 대기중인 무기 액터
	// NOTE: 서버에서 이 값을 write할 경우 Setter를 사용하는 것이 강력하게 권장됨
	UPROPERTY(ReplicatedUsing = OnRep_CurrWeaponActor)
	TObjectPtr<class AWieldItem> CurrWeaponActor = nullptr;
};
