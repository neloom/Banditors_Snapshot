// Copyright (C) 2024 by Haguk Kim


#include "EquipSystem.h"
#include "WieldItem.h"
#include "FPSCharacter.h"
#include "TRHUDWidget.h"
#include "RecoilAnimationComponent.h"

// Sets default values for this component's properties
UEquipSystem::UEquipSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UEquipSystem::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UEquipSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

TObjectPtr<AGameCharacter> UEquipSystem::GetTROwner()
{
	AGameCharacter* Owner = Cast<AGameCharacter>(GetOwner());
	if (Owner) 
	{
		return Owner;
	}
	return nullptr;
}

bool UEquipSystem::Server_SwitchWeaponSlotTo(int32 SlotIdx)
{
	// Out of range
	if (SlotIdx < 0 || SlotIdx > SlotCount) return false;

	// No owner
	AGameCharacter* TROwner = GetTROwner();
	if (!IsValid(TROwner)) return false;

	// 이미 해당 슬롯을 선택했고 적절한 아이템이 생성 완료된 경우 스킵한다
	// NOTE: 그냥 다시 재처리해도 지장은 없으나, 성능 최적화를 위함임
	// NOTE: 슬롯 번호가 같지만 액터가 소환이 안된 경우(레벨 트랜지션 등)가 있을 수 있기 때문에 이렇게 두 번 확인하는 것이 필수적임
	UInvObject* OriginInvObj = GetInvObjOfSlot(SlotIdx);
	if (SlotIdx == CurrentSlot)
	{
		if (OriginInvObj && IsValid(CurrWeaponActor) && CurrWeaponActor->GetInvObject() == OriginInvObj)
		{
			return true;
		}
	}

	// 사용 대기 상태 해제 (장착 중인 아이템이 있을 경우 해당 액터 파괴)
	Server_RetrieveCurrWeapon();

	// 슬롯 변경
	SwitchSlotTo(SlotIdx);

	// 해당 슬롯 무기를 구한다
	UInvObject* NewInvObj = GetInvObjOfCurrSlot();
	if (NewInvObj)
	{
		// 액터 생성
		// 이때 슬롯에는 여전히 InvObject가 남아있다
		ABaseItem* SpawnedItem = NewInvObj->GenerateAndSpawnItem(GetTROwner(), GetTROwner()->GetHandPointInfo().Get<0>()/*임시로 그랩 위치에 생성*/, GetTROwner()->GetActorRotation(), FActorSpawnParameters());
		Server_SetCurrWeaponActor(Cast<AWieldItem>(SpawnedItem));
		if (!CurrWeaponActor)
		{
			UE_LOG(LogTemp, Fatal, TEXT("Server_SwitchWeaponSlotTo() - Something went wrong during AWieldItem generation: %s"), *(SpawnedItem->GetName()));
			return false;
		}

		// 전 호스트에 WeaponState를 먼저 공유한다
		// 이 정보는 액터 레플리케이션보다 먼저 도착할 수 있는데, 이 경우 뒤이어 액터 정보가 도착해 애니메이션이 먼저 재생되고 무기가 붙는 연출로 처리된다
		// 만약 액터 정보가 먼저 도착할 경우에는 무기가 팔에 먼저 붙고 그걸 들어올리는 형태로 조금 더 자연스럽게 처리된다
		if (CurrWeaponActor->ShouldHoldWithBothArms())
		{
			Multicast_OnSwitchWeapon(EHumanoidWeaponState::HEAVY);
			GetTROwner()->Server_SwitchAnimClassLayerRPC(EAnimClassType::AC_HEAVYWEAPON);
		}
		else
		{
			Multicast_OnSwitchWeapon(EHumanoidWeaponState::LIGHT);
			GetTROwner()->Server_SwitchAnimClassLayerRPC(EAnimClassType::AC_LIGHTWEAPON);
		}

		// 공통로직 실행
		ProcessWeaponAttachment();
		ProcessWeaponRecoilInit();
	}
	else
	{
		// 무기가 없을 경우
		Multicast_OnSwitchWeapon(EHumanoidWeaponState::NONE);
		GetTROwner()->Server_SwitchAnimClassLayerRPC(EAnimClassType::AC_UNARMED);
	}

	// 서버의 경우 수동 호출
	Local_OnCurrWeaponActorUpdated();
	return true;
}

void UEquipSystem::Multicast_OnSwitchWeapon_Implementation(EHumanoidWeaponState WeaponState)
{
	// WeaponState 설정
	GetTROwner()->WeaponEquipState = WeaponState;

	// 슬롯에 장착된 무기의 애니메이션 재생
	GetTROwner()->PlayDeployAnimMontage();
}

void UEquipSystem::Server_RetrieveCurrWeapon()
{
	// 기존에 들고 있는 무기가 있었을 경우 집어넣는다 (액터만 파괴)
	if (CurrWeaponActor)
	{
		// 공통로직 실행
		ProcessWeaponDetachment();

		// 아이템 파괴
		CurrWeaponActor->OnItemRetrieve(this);
		Server_SetCurrWeaponActor(nullptr);
	}

	// 서버의 경우 수동 호출
	Local_OnCurrWeaponActorUpdated();
}

bool UEquipSystem::TryEquipItemAt(UInvObject* InvObj, int32 SlotIdx)
{
	if (GetInvObjOfSlot(SlotIdx)) return false; // 위치 중복 허용 X
	TryAddInvObjToSlot(InvObj, SlotIdx);
	return true;
}

void UEquipSystem::ProcessWeaponAttachment()
{
	if (CurrWeaponActor)
	{
		// 피직스, 콜리전 해제
		// NOTE: 소켓에 부착하기 전에 먼저 피직스를 해제해주어야 정상적으로 처리됨 (콜리전은 무관)
		// 이는 언리얼에서 두 피직스 오브젝트의 Attach를 Weld가 아닌 이상 허용하지 않기 때문임
		// SceneComponent.cpp 참고
		CurrWeaponActor->SetItemPhysicsTo(false);
		CurrWeaponActor->SetActorEnableCollision(false); // 모든 콜리전 해제

		// 무기 메쉬에 부착
		GetTROwner()->AttachItem(CurrWeaponActor);
	}
}

void UEquipSystem::ProcessWeaponDetachment()
{
	if (CurrWeaponActor)
	{
		// 무기 메쉬에서 탈착
		GetTROwner()->DetachItem(CurrWeaponActor);

		// 피직스, 콜리전 기본값으로 되돌림
		CurrWeaponActor->SetItemPhysicsBackToDefault();
		CurrWeaponActor->SetItemCollisionWithPawnToDefault();
	}
}

void UEquipSystem::ProcessWeaponRecoilInit()
{
	// 반동 초기화
	AGameCharacter* OwnerChar = GetTROwner();
	if (OwnerChar && CurrWeaponActor)
	{
		OwnerChar->Local_InitRecoil(CurrWeaponActor->RecoilAnimData, CurrWeaponActor->RecoilRate, CurrWeaponActor->RecoilBurst);
	}
}

bool UEquipSystem::CanMoveInvObj(UInvObject* InvObj)
{
	if (!Super::CanMoveInvObj(InvObj)) return false;

	// LEGACY: CurrentSlot에 위치한 InvObj는 움직이지 못하게 만들기
	/*if (GetInvObjAtXY(CurrentSlot, 0) == InvObj) 
	{
		return false;
	}*/
	return true;
}

bool UEquipSystem::Server_CanAcceptInvObj(UInvObject* InvObj)
{
	if (!InvObj || !InvObj->GetBaseItemClass())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_CanAcceptInvObj - InvObj or its base class is null!"));
		return false;
	}
	if (InvObj->GetBaseItemClass()->IsChildOf<AWieldItem>()) return true;
	return false;
}

AWieldItem* UEquipSystem::GetCurrWeaponActor()
{
	return CurrWeaponActor.Get();
}

void UEquipSystem::OnRep_CurrWeaponActor()
{
	if (CurrWeaponActor)
	{
		// 공통로직 실행
		ProcessWeaponAttachment();
		ProcessWeaponRecoilInit();
	}
	else
	{
		ProcessWeaponDetachment();
	}

	Local_OnCurrWeaponActorUpdated();
}

void UEquipSystem::Server_SetCurrWeaponActor(AWieldItem* NewWeapon)
{
	AGameCharacter* CharOwner = Cast<AGameCharacter>(GetOwner());
	if (!CharOwner || !CharOwner->HasAuthority()) return;

	AWieldItem* PrevWeapon = CurrWeaponActor; // nullptr일 수 있음
	CurrWeaponActor = NewWeapon;
	Server_OnCurrWeaponActorChanged(PrevWeapon);
	return;
}

void UEquipSystem::Server_OnCurrWeaponActorChanged(AWieldItem* OldWeapon)
{
	AGameCharacter* CharOwner = Cast<AGameCharacter>(GetOwner());
	if (!CharOwner || !CharOwner->HasAuthority()) return;

	// 스테이터스 이펙트 제거 및 추가
	if (OldWeapon && OldWeapon->GetInvObject())
	{
		CharOwner->Server_RemoveStatEffect(OldWeapon->GetInvObject()->CachedWielderStatEffect.Get());
	}
	if (CurrWeaponActor)
	{
		// 스테이터스 이펙트
		UInvObject* CurrInvObj = CurrWeaponActor->GetInvObject();
		if (CurrInvObj && CurrInvObj->bApplyStatusEffectToWielder)
		{
			UStatusEffect* WielderStatusEffect = CharOwner->Server_GenerateAndAddStatEffect(CurrInvObj->WielderStatusEffectData, CharOwner /* 스스로 무기를 바꿈 */);
			if (WielderStatusEffect)
			{
				CurrInvObj->CachedWielderStatEffect = MakeWeakObjectPtr<UStatusEffect>(WielderStatusEffect);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Server_OnCurrWeaponActorChanged - New UStatusEffect instance was not generated. This could happen in certain scenarios(e.g. item grants temporary stateff after equip), but most likely something has gone wrong."));
			}
		}
	}
}

void UEquipSystem::Server_OnInventoryContentChanged()
{
	AGameCharacter* TROwner = GetTROwner();
	if (TROwner)
	{
		if (!bIsImportingInventory)
		{
			// NOTE: 장착무기 상태의 업데이트는 게임플레이 로직의 일부분이기 때문에 인벤토리가 임포트 중인 경우 사용할 수 없다
			TROwner->SwitchWeaponSlotTo(this->GetCurrentSlot());
		}
	}
}

void UEquipSystem::Local_OnCurrWeaponActorUpdated()
{
	AGameCharacter* GameChar = GetTROwner();
	if (GameChar && GameChar->Local_GetBoundHUDWidget().IsValid())
	{
		GameChar->Local_GetBoundHUDWidget()->UpdateSlot();
		GameChar->Local_GetBoundHUDWidget()->UpdateAmmo();
	}
}

