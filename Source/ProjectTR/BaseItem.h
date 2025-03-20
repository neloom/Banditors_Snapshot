// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Engine/ActorChannel.h"
#include "TREnums.h"
#include "BaseItem.generated.h"

USTRUCT(BlueprintType)
struct FItemAttribute
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttrName = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttrValue = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemAttrType AttrType = EItemAttrType::IAT_NEUTRAL_NORMAL;

public:
	FItemAttribute() {}
	FItemAttribute(FString Name, FString Value, EItemAttrType Type)
	{
		AttrName = Name;
		AttrValue = Value;
		AttrType = Type;
	}

	// 타입순 정렬
	bool operator<(const FItemAttribute& Other) const
	{
		return AttrType < Other.AttrType;
	}
};

UCLASS()
class PROJECTTR_API ABaseItem : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseItem();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 개별 액터에서 PostInitializeComponents 대신 이 함수를 오버라이드해 사용한다
	virtual void OnPostInitializeComponents();
public:
	virtual void Tick(float DeltaTime) override;

	// 아이템 캐싱 및 아우터 변경 작업이 필요할 경우 호출한다
	// NOTE: 아이템 액터를 Destory할 경우, 만약 아이템 정보가 인벤토리(EquSys 등)에 남을 경우 반드시 이 함수를 호출해주어야 한다
	// e.g. 무기 슬롯 변경 시 Deploy된 무기 Retrieve, 아이템 Pickup 등
	// NOTE: 이 함수는 아이템 정보를 변형시킬 수 있으므로 특수한 경우를 제외하면 가급적 아이템 파괴 직전에만 호출하는 것이 권장된다
	void CacheBeforeDestruction(UObject* NewOuter);

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void ThrowInDirection(const FVector& Direction);

#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ABaseItem, ItemData);
		DOREPLIFETIME(ABaseItem, InvObject);
		DOREPLIFETIME(ABaseItem, MeshComponent);
	}
#pragma endregion

#pragma region /** Components */
public:
	// 아이템 리치 판정용 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<class UBoxComponent> ReachComponent = nullptr;

	// 아이템 메쉬 (콜리전, 피직스)
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Mesh")
	TObjectPtr<UMeshComponent> MeshComponent = nullptr;

	/* Getters */
	// 메쉬
	UMeshComponent* GetMeshComponent() const { return MeshComponent; }

	// 피직스 연산 시 사용할 컴포넌트
	UPrimitiveComponent* GetPhysComponent() const { return Cast<UPrimitiveComponent>(GetRootComponent()); }

protected:
	// 루트를 메쉬 컴포넌트로 변경한다
	void SetRootToMeshComponent();
#pragma endregion

#pragma region /** Initializer */
protected:
	// 루트 컴포넌트 초기화
	// 루트가 메쉬 컴포넌트이던, 박스 컴포넌트이던 무엇이던간에
	// 루트에 대해 반드시 실행해야 하는 로직을 정의한다
	void InitRootComp(UPrimitiveComponent* Component);

	// 메쉬의 값들을 초기화한다
	// BaseItem은 기본적으로 메쉬를 루트로 사용하지만,
	// 만약 메쉬가 루트가 아닌 경우 이 함수를 오버라이드 해 재설정해주어야 한다
	virtual void InitMeshComp(UPrimitiveComponent* Component);

	// 아이템 리치 판정 컴포넌트 초기화
	virtual void InitReachComp(UPrimitiveComponent* Component);
#pragma endregion

#pragma region /** Collision */
protected:
	// 리치 콜리전 시 로직
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

protected:
	// 아이템이 기본적으로 중력을 연산해야 하는지 여부. 현재 상태와 다를 수 있음
	// e.g. 무기 장착을 할 경우 중력 연산이 일시적으로 중단됨
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	int DefaultItemCollisionWithPawn = ECollisionResponse::ECR_Ignore;

public:
	// 모든 피직스 콜리전을 해제한다
	void DisableItemCollision();

	// 함수를 호출한 시점에서 이 아이템이 플레이어 폰 및 봇 폰과 블로킹 하는지 여부
	bool DoesItemBlockWithPawn() const;

	// 이 콜리전 및 오버랩 설정을 기본 설정으로 되돌린다
	void SetItemCollisionWithPawnToDefault();

	// 이 아이템의 콜리전 및 오버랩 설정을 주어진 값으로 설정한다
	void SetItemCollisionWithPawnTo(ECollisionResponse ColResponse);

protected:
	// 리치 컴포넌트의 크기를 동적으로 현재 메쉬에 맞게 수정한다
	virtual void ResizeReachCompToMatchItem();

protected:
	// 리치 컴포넌트의 크기에 아이템 메시에 대해 얼마만큼의 여백을 줄 것인지 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ReachCompMargin = 10.0f;
#pragma endregion

#pragma region /** Physics */
/* Gravity */
protected:
	// 아이템이 기본적으로 중력을 연산해야 하는지 여부. 현재 상태와 다를 수 있음
	// e.g. 무기 장착을 할 경우 중력 연산이 일시적으로 중단됨
	UPROPERTY(EditDefaultsOnly, Category = "Physics")
	bool bShouldItemSimulateGravity = true;

public:
	// 함수를 호출한 시점에서 이 아이템이 중력을 연산하고 있는지 여부
	bool IsItemSimulatingGravity() const;

	// 이 아이템의 중력 설정을 기본 중력 설정으로 되돌린다
	void SetItemGravityBackToDefault();

	// 이 아이템의 중력 설정을 주어진 값으로 설정한다
	void SetItemGravityTo(bool bGravity);

/* Physics */
protected:
	// 아이템이 기본적으로 물리 연산을 하는지 여부. 현재 상태와 다를 수 있음
	UPROPERTY(EditDefaultsOnly, Category = "Physics")
	bool bShouldItemSimulatePhysics = true;

public:
	// 함수를 호출한 시점에서 이 아이템이 물리 연산을 하고 있는지 여부
	bool IsItemSimulatingPhysics() const;

	// 이 아이템의 물리 연산 설정을 기본 설정으로 되돌린다
	void SetItemPhysicsBackToDefault();

	// 이 아이템의 물리 연산 설정을 주어진 값으로 설정한다
	void SetItemPhysicsTo(bool bPhysics);
#pragma endregion

#pragma region /** Item Data */
protected:
	// 아이템 데이터 클래스
	UPROPERTY(EditAnywhere, Category = "Item Data")
	TSubclassOf<class UItemData> ItemDataClass = nullptr;

	// 아이템 데이터
	UPROPERTY(BlueprintReadOnly, Replicated)
	TObjectPtr<class UItemData> ItemData = nullptr;

protected:
	// 이 아이템에 대응되는 아이템 데이터 객체를 생성한다
	void GenerateItemData(UObject* Outer);

public:
	// 인자로 전달된 ItemData에 할당된 정보를 기반으로 이 아이템의 정보를 복구한다
	// 성공 여부를 반환한다
	virtual bool RestoreFromItemData(class UItemData* Data);

	// ItemData 등록
	void SetItemData(class UItemData* Data);

	// 이 아이템의 현재 정보를 ItemData에 캐싱한다
	void CacheToItemData() const;
#pragma endregion

#pragma region /** Interface */
protected:
	// 아이템 설명을 생성한다
	// 정적으로 바인딩 되어있을 경우 그대로 그 값을 사용한다
	virtual void GenerateInvObjDescription();

public:
	// 최초 1회 아이콘을 생성한다
	// 이 함수는 새 스테이지 액터를 생성하기 때문에 일반적으로 처음 1회에만 사용되지만,
	// 레벨 트랜지션과 같이 스테이지 액터가 유효하지 않은 경우 사용될 수 있다
	// 이미 스테이지 액터가 존재하는 상황에서의 아이콘 갱신은 Host_ProcessRefreshIcon을 사용해야 한다
	UFUNCTION()
	void Server_InitializeIcon();

public:
	// 이 변수들은 인벤토리 오브젝트에 전달할 값을 임시로 담거나, 혹은 BP에 노출시켜 수동으로 입력하기 위한 임시 값으로
	// 실제 게임플레이 상에서 아이템 설명을 가져오기 위해서는 InvObject를 사용해야 한다
	UPROPERTY(EditDefaultsOnly)
	FString TempItemDesc = "";

	UPROPERTY(EditDefaultsOnly)
	TArray<FItemAttribute> TempItemAttributesForUI;

	// 아이콘 생성 시 아이템을 얼마나 기울인 상태에서 생성할 것인지 지정
	UPROPERTY(EditAnywhere)
	FRotator IconDisplayRotation = FRotator();

	// 최초 생성 시 아이콘을 초기해야하는지 여부
	UPROPERTY(EditDefaultsOnly)
	bool bShouldInitializeIcon = true;

	// 서버에서 아이콘 생성(갱신) 요청이 1회 이상 처리되었는지 여부
	// 서버에서 처리되었다면 클라도 처리된 것으로 간주
	bool bServer_HasInitializedIcon = false;
#pragma endregion

#pragma region /** Inventory */
protected:
	// InvObject 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<class UInvObject> InvObjectClass = nullptr;

	// InvObject
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Inventory")
	TObjectPtr<class UInvObject> InvObject = nullptr;

public:
	// 이 아이템에 대응되는 InvObject를 생성하고 생성된 InvObject에 ItemData를 등록한다
	void GenerateInvObject(UObject* Outer);

	// 아이템 픽업 시 호출; 성공적으로 인벤토리에 추가되었는지 여부 반환
	bool OnItemPickup(class UInventoryComponent* InvComp);

	// InvObject Setter
	void SetInvObject(class UInvObject* InvObj);

	// InvObject Getter
	TObjectPtr<class UInvObject> GetInvObject();

protected:
	// InvObject가 등록되있는지 확인한다
	void VerifyInvObject();

	// 기본값 InvObject를 생성한다
	UInvObject* CreateDefaultInvObject();
#pragma endregion

#pragma region /** Equipments */
public:
	// 아이템 장착 시도 시 호출
	virtual bool OnItemEquip(class UEquipSystem* EquSys, int32 SlotIdx);

	// 이 아이템의 물리적 크기와 유사한 값을 반환한다
	virtual FVector GetEstimatedItemSize();
#pragma endregion

#pragma region /** Attachment */
/* Attachment */
protected:
	// 장착자 메쉬에 탈부착할 때의 룰 지정
	FAttachmentTransformRules EquipAttachRule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
	FDetachmentTransformRules EquipDetachRule = FDetachmentTransformRules::KeepWorldTransform; // Default

	// 캐릭터 메쉬에 부착할 때의 이 아이템의 상대적 값들 지정
	// 이 값들은 BP에서 직접 세부 조정을 해줘야 함
	// 위치
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attach")
	FVector AttachRelativeLocation;

	// 회전
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attach")
	FRotator AttachRelativeRotation;

	// 캐릭터 메쉬에 부착할 때 캐릭터의 어느 부위에 부착할지 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attach")
	ECharacterParts AttachPart = ECharacterParts::ECP_PrimaryWield;

	// 현재 이 아이템이 부착되어있는 대상의 소켓의 이름
	FName AttachedSocketName;

public:
	// Getters
	const FAttachmentTransformRules GetEquipAttachRule() { return EquipAttachRule; }
	const FDetachmentTransformRules GetEquipDetachRule() { return EquipDetachRule; }
	const FVector GetAttachRelativeLocation() { return AttachRelativeLocation; }
	const FRotator GetAttachRelativeRotation() { return AttachRelativeRotation; }
	const ECharacterParts GetCharacterAttachPartName() { return AttachPart; }
#pragma endregion

#pragma region /** Render */
public:
	// 이 아이템의 렌더링 여부를 변경하는 걸 요청한다
	// 동적으로 생성된 메쉬가 있을 경우, 모든 메쉬들의 레플리케이션이 처리 완료된 시점에 처리해야 한다
	virtual void RegisterVisibility(bool bVisibility);

	// 이 아이템의 렌더링 여부를 설정한다
	// 오버라이드 시 부모 함수를 콜해야 한다
	virtual void SetItemVisibility(bool bVisibility);

protected:
	// RegisterVisibility 호출 이후 아직 SetItemVisibility가 호출되지 않았을 경우 true로 설정한다
	bool bShouldRenewItemVisibility = false;

	// 이 아이템의 가시성 여부
	// NOTE: 이 값은 클라이언트의 동적 메쉬에서는 즉각적으로 적용되지 않을 수 있다
	// 이 경우 동적메쉬들의 레플리케이션이 끝나는 대로 값이 적용된다
	bool bItemVisibility = true;
#pragma endregion

#pragma region /** Debug */
public:
	void PrintItemDataTestValue();
#pragma endregion
};
