// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "PickupItem.h"
#include "RecoilAnimationComponent.h"
#include "WieldItem.generated.h"

class UActItemComponent;

/**
 * 
 */
UCLASS()
class PROJECTTR_API AWieldItem : public APickupItem
{
	GENERATED_BODY()

public:
	AWieldItem();
	virtual void OnDeployerStatChanged();

protected:
	virtual void BeginPlay() override;

#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AWieldItem, PrimaryActComponent);
		DOREPLIFETIME(AWieldItem, SecondaryActComponent);
	}
#pragma endregion

#pragma region /** Action */
public:
	// 컴포넌트 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	TSubclassOf<UActItemComponent> PrimaryActCompClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	TSubclassOf<UActItemComponent> SecondaryActCompClass = nullptr;

	// 컴포넌트
	UPROPERTY(Replicated)
	TObjectPtr<UActItemComponent> PrimaryActComponent = nullptr;

	UPROPERTY(Replicated)
	TObjectPtr<UActItemComponent> SecondaryActComponent = nullptr;
#pragma endregion

#pragma region /** Equipments */
protected:
	// 장착 가능 여부
	bool bCanEquip = true;

	// 양손 파지를 해야 하는지 여부; 애니메이션 모션을 결정짓는 데 사용된다
	bool bShouldHoldWithBothArms = false;

public:
	// 장착 시 로직
	virtual bool OnItemEquip(class UEquipSystem* EquSys, int32 SlotIdx) override;

	// 디플로이 해제 시 로직 (무기를 바꾸는 경우)
	// 장착 해제와는 별도의 로직
	void OnItemRetrieve(class UEquipSystem* EquSys);

	// 성공적으로 트리거를 마쳤을 때 아이템에 대해 수행하는 로직
	// 문제가 발견되었을 경우 false를 반환한다
	virtual bool OnItemTriggerProcessed(UActItemComponent* ActComp);

	// Getters
	AGameCharacter* GetItemDeployer();
	const bool CanEquip() { return bCanEquip; }
	const bool ShouldHoldWithBothArms() { return bShouldHoldWithBothArms; }
#pragma endregion

#pragma region /** Recoil */
// NOTE: 반동 관련 데이터들은 로컬 값이다
public:
	// 현재 사용중인 반동 정보
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	FRecoilAnimData RecoilAnimData;

	// 현재 사용중인 반동 rate (Shots per minute)
	float RecoilRate = 0.0f;

	// 현재 사용중인 반동 버스트
	int RecoilBurst = 0; // TODO

public:
	// 이 아이템이 사용할 기본 반동 프로퍼티들
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UStoredData* LightRecoilAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UStoredData* HeavyRecoilAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UCurveVector* LightLocVector = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UCurveVector* LightRotVector = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UCurveVector* HeavyLocVector = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UCurveVector* HeavyRotVector = nullptr;
#pragma endregion
};
