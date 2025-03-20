// Copyright (C) 2024 by Haguk Kim


#include "WieldItem.h"
#include "GameCharacter.h"
#include "ActItemComponent.h"
#include "EquipSystem.h"
#include "ItemData.h"
#include "TRMacros.h"
#include "StoredData.h"
#include "Curves/CurveVector.h"

AWieldItem::AWieldItem()
{
#pragma region /** Recoil Initialization */
	// 반동 기본값 설정
	// 에셋
	static ConstructorHelpers::FObjectFinder<UStoredData> DataAssetFinderLight(TEXT(DEFAULT_RECOIL_LIGHT_ASSET));
	if (DataAssetFinderLight.Succeeded())
	{
		LightRecoilAsset = DataAssetFinderLight.Object;
	}
	static ConstructorHelpers::FObjectFinder<UStoredData> DataAssetFinderHeavy(TEXT(DEFAULT_RECOIL_HEAVY_ASSET));
	if (DataAssetFinderHeavy.Succeeded())
	{
		HeavyRecoilAsset = DataAssetFinderHeavy.Object;
	}

	// Location 커브
	static ConstructorHelpers::FObjectFinder<UCurveVector> CurveVecFinderLightLoc(TEXT(DEFAULT_RECOIL_LIGHT_LOC));
	if (CurveVecFinderLightLoc.Succeeded())
	{
		LightLocVector = CurveVecFinderLightLoc.Object;
	}
	static ConstructorHelpers::FObjectFinder<UCurveVector> CurveVecFinderHeavyLoc(TEXT(DEFAULT_RECOIL_HEAVY_LOC));
	if (CurveVecFinderHeavyLoc.Succeeded())
	{
		HeavyLocVector = CurveVecFinderHeavyLoc.Object;
	}

	// Rotation 커브
	static ConstructorHelpers::FObjectFinder<UCurveVector> CurveVecFinderLightRot(TEXT(DEFAULT_RECOIL_LIGHT_ROT));
	if (CurveVecFinderLightRot.Succeeded())
	{
		LightRotVector = CurveVecFinderLightRot.Object;
	}
	static ConstructorHelpers::FObjectFinder<UCurveVector> CurveVecFinderHeavyRot(TEXT(DEFAULT_RECOIL_HEAVY_ROT));
	if (CurveVecFinderHeavyRot.Succeeded())
	{
		HeavyRotVector = CurveVecFinderHeavyRot.Object;
	}

	check(LightRecoilAsset);
	check(LightLocVector);
	check(LightRotVector);
	check(HeavyRecoilAsset);
	check(HeavyLocVector);
	check(HeavyRotVector);

	// PRAS - 지정된 반동 정보가 없을 경우 기본값(Light) 사용
	if (!RecoilAnimData.StoredData)
	{
		RecoilAnimData.StoredData = LightRecoilAsset;
	}
	if (!RecoilAnimData.SingleLoc)
	{
		RecoilAnimData.SingleLoc = LightLocVector;
	}
	if (!RecoilAnimData.SingleRot)
	{
		RecoilAnimData.SingleRot = LightRotVector;
	}
#pragma endregion
}

void AWieldItem::OnDeployerStatChanged()
{
	// 필요 시 오버라이드
}

void AWieldItem::BeginPlay()
{
	Super::BeginPlay();
	
	// 액션 컴포넌트 초기화
	if (IsValid(PrimaryActCompClass))
	{
		PrimaryActComponent = NewObject<UActItemComponent>(this, PrimaryActCompClass);
		if (PrimaryActComponent)
		{
			PrimaryActComponent->RegisterComponent();
		}
	}

	if (IsValid(SecondaryActCompClass))
	{
		SecondaryActComponent = NewObject<UActItemComponent>(this, SecondaryActCompClass);
		if (SecondaryActComponent)
		{
			SecondaryActComponent->RegisterComponent();
		}
	}
}

bool AWieldItem::OnItemEquip(UEquipSystem* EquSys, int32 SlotIdx)
{
	if (IsValid(EquSys))
	{
		if (EquSys->TryAddInvObjToSlot(this->InvObject, SlotIdx))
		{
			if (HasAuthority())
			{
				CacheBeforeDestruction(EquSys->GetOwner());
				Destroy();
			}
			return true;
		}
	}
	return false;
}

void AWieldItem::OnItemRetrieve(UEquipSystem* EquSys)
{
	if (IsValid(EquSys))
	{
		if (HasAuthority())
		{
			// 이미 인벤토리에 InvObject가 저장되어있는 상태이므로 캐싱 후 액터만 Destroy한다
			CacheBeforeDestruction(EquSys->GetOwner());
			Destroy();
		}
		return;
	}
}

bool AWieldItem::OnItemTriggerProcessed(UActItemComponent* ActComp)
{
	if (!ActComp || (ActComp != PrimaryActComponent && ActComp != SecondaryActComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("OnItemTriggerProcessed - ActComponent is invalid!"));
		return false;
	}
	// 기본적으로 아무 동작도 수행하지 않는다
	// 필요 시 오버라이드
	return true;
}

AGameCharacter* AWieldItem::GetItemDeployer()
{
	if (!InvObject)
	{
		UE_LOG(LogTemp, Error, TEXT("GetItemDeployer - InvObject is invalid!"));
		return nullptr;
	}

	// 클라에서 호출 시 레플리케이션 상태에 따라 결과가 늦을 수 있다
	AGameCharacter* ItemOwner = Cast<AGameCharacter>(InvObject->GetOuter());
	if (ItemOwner && ItemOwner->EquipSystem && ItemOwner->EquipSystem->GetCurrWeaponActor() == this)
	{
		return ItemOwner;
	}
	return nullptr;
}
