// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Containers/Map.h"
#include "Net/UnrealNetwork.h"
#include "ReplicatedObject.h"
#include "StatusEffect.h"
#include "InvObject.generated.h"

USTRUCT(BlueprintType)
struct FInvObjSize
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 X = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Y = 0;
};

/**
 * InvObject는 ABaseItem의 생성에 의해서 부가적으로 생성하는 것을 권장하며, UInvObject 객체를 직접 생성하는 행위는 지양한다.
 * 액터가 필요하지 않은 경우에도 (e.g. 창고 안에 들어있는 상태의 아이템을 생성) 우선 액터를 생성하고 그 후 액터로부터 Detach하는 형태로 처리한다.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTTR_API UInvObject : public UReplicatedObject
{
	GENERATED_BODY()

public:
	UInvObject();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UInvObject, ItemName);
		DOREPLIFETIME(UInvObject, ItemDesc);
		DOREPLIFETIME(UInvObject, ItemAttributesForUI);
		DOREPLIFETIME(UInvObject, CurrIconStageActor);
	}

public:
	// 이 InvObject에 대응되는 아이템을 생성하고 생성된 아이템에 ItemData를 등록한다
	class ABaseItem* GenerateAndSpawnItem(UObject* Outer, FVector Location, FRotator Rotation, FActorSpawnParameters Params, bool bRestoreUsingItemData = true);

	// 이 오브젝트 및 이 오브젝트 하위의 모든 오브젝트들에 대해 수동으로 GC 타깃팅을 설정 혹은 해제한다
	// NewOuter가 nullptr가 아닌 경우 아우터도 수동으로 변경한다
	virtual void ChangeRootSetRecursive(bool bAddRoot, UObject* NewOuter = nullptr);

#pragma region /** Gameplay */
/* Status Effects */
// 아이템 스테이터스 이펙트 인스턴스는 구조체 값을 기반으로 생성된다
// 이 구조체 값은 정적으로 에디터 상에서 값을 지정할 수도 있고, 필요 시 동적으로 결정되도록 만들 수 있다
public:
	// 보유자(InventoryComp 혹은 그 상속 컴포넌트에 추가된 경우 컴포넌트의 오너)에게 부여할 스테이터스 이펙트 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	FStatEffectGenInfo OwnerStatusEffectData;

	// 사용자(Deploy한 오너)에게 부여할 스테이터스 이펙트 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	FStatEffectGenInfo WielderStatusEffectData;

	// 보유자에게 지정된 스테이터스 이펙트를 부여할지 여부
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	bool bApplyStatusEffectToOwner = false;

	// 사용자에게 지정된 스테이터스 이펙트를 부여할지 여부
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	bool bApplyStatusEffectToWielder = false;

	// 이 오브젝트가 어떤 캐릭터에게 스테이터스 이펙트를 부여중인 경우 해당 인스턴스를 캐싱
	TWeakObjectPtr<class UStatusEffect> CachedOwnerStatEffect = nullptr;
	TWeakObjectPtr<class UStatusEffect> CachedWielderStatEffect = nullptr;

/* Shop */
protected:
	// 가격
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	int32 Price = 1;

	// 판매 가능 여부
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	bool bMarketable = true;

public:
	// Getters
	const int32 Local_GetPrice() const { return Price; }
	const bool Local_IsMarketable() const { return bMarketable; }
#pragma endregion

#pragma region /** Item Data */
protected:
	// 아이템 데이터
	// 아이템 액터 생성 시 초기화
	UPROPERTY(BlueprintReadOnly, Category = "Item Data")
	TObjectPtr<class UItemData> ItemData = nullptr;

	// 대응되는 아이템 클래스
	// 아이템 액터 생성 시 초기화
	UPROPERTY()
	TSubclassOf<class ABaseItem> BaseItemClass = nullptr;

public:
	/* Setters */
	void SetItemData(class UItemData* Data);
	void SetBaseItemClass(TSubclassOf<class ABaseItem> Class);

	/* Getters */
	class UItemData* GetItemData() { return ItemData; }
	TSubclassOf<class ABaseItem> GetBaseItemClass() { return BaseItemClass; }
#pragma endregion

#pragma region /** Logic */
/* Inventory grid */
protected:
	// 인벤토리 격자에서 차지하는 X 크기 (고정)
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 InvXSize = 1;

	// 인벤토리 격자에서 차지하는 Y 크기 (고정)
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 InvYSize = 1;

public:
	// 이 InvObject의 격자 크기를 X, Y 순으로 반환한다
	UFUNCTION(BlueprintCallable)
	FInvObjSize GetDimensions() const;
#pragma endregion

#pragma region /** Interface */
/* Description */
// 이 값들은 오로지 UI 표기를 위해서만 사용된다
protected:
	UPROPERTY(Replicated, EditDefaultsOnly)
	FString ItemName = "";

	UPROPERTY(Replicated)
	FString ItemDesc = "";

	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<FItemAttribute> ItemAttributesForUI;

public:
	UFUNCTION(BlueprintCallable)
	const FString GetInvObjDesc() const { return ItemDesc; }
	void SetInvObjDesc(FString Desc) { ItemDesc = Desc; }

	UFUNCTION(BlueprintCallable)
	const FString GetInvObjName() const { return ItemName; }
	void SetInvObjName(FString Name) { ItemName = Name; }

	UFUNCTION(BlueprintCallable)
	const TArray<FItemAttribute> GetInvObjAttr() const { return ItemAttributesForUI; }
	void SetInvObjAttr(const TArray<FItemAttribute>& Attr) { ItemAttributesForUI = Attr; }

/* Icon */
protected:
	// 다이나믹 매터리얼 생성에 사용할 베이스 매터리얼
	TObjectPtr<UMaterial> BaseIconMaterial = nullptr;

	// 아이콘 매터리얼
	UPROPERTY(EditDefaultsOnly, Category = "Icon")
	TObjectPtr<UMaterialInstance> IconMat = nullptr;
	
	// 아이콘 매터리얼이 Dynamic한지 여부; 렌더타깃 등의 기능을 사용하기 위해서는 Dynamic material이 필요하다
	// 이 값은 캐싱을 통해 보존되지 않는다
	bool bIsIconMatDynamic = false;

	// 이 값이 참일 경우 매터리얼 인스턴스를 직접 바운딩해 아이콘으로 그대로 사용한다
	// 이 경우 가로세로 픽셀 크기와 비율을 고려해야 하기 때문에 권장되는 방법은 아니다
	UPROPERTY(EditDefaultsOnly, Category = "Icon")
	bool bUseStaticBoundIcon = false;

public:
	// 현재 아이콘 매터리얼 Getter
	UFUNCTION(BlueprintCallable)
	UMaterialInstance* GetCurrIcon() const;

	// 아이콘 매터리얼 Getter
	FORCEINLINE UMaterialInstance* GetIcon() const;

	// 동적으로 이 오브젝트에 맞는 아이콘을 생성한다
	// NOTE: 동적으로 메쉬 정보가 변하는 아이템의 경우 아이콘 생성 전에 해당 정보들이 ItemData에 캐싱되어있어야 한다
	void Server_RequestUpdateIcon();

	// 주어진 액터를 기반으로 렌더타겟을 설정해 매터리얼을 추출하고 InvObject의 아이콘으로 설정한다
	void Host_ProcessRefreshIcon(class AIconStageActor* TargetActor);

/* IconStage */
protected:
	// 현재 바인딩된 아이콘의 원본 액터
	// nullptr일 수 있다
	UPROPERTY(ReplicatedUsing = OnIconStageActorRepl)
	class AIconStageActor* CurrIconStageActor = nullptr;

public:
	void SetCurrIconStageActor(class AIconStageActor* IconStageActor);
	class AIconStageActor* GetCurrIconStageActor() { return CurrIconStageActor; }

protected:
	UFUNCTION()
	void OnIconStageActorRepl();
#pragma endregion
};
