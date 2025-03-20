// Copyright (C) 2024 by Haguk Kim


#include "BaseItem.h"
#include "TRMacros.h"
#include "CustomUtil.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ItemData.h"
#include "InvObject.h"
#include "InventoryComponent.h"
#include "FPSCharacter.h"
#include "WieldItem.h"
#include "TRUtils.h"
#include "TRStructs.h"

// Sets default values
ABaseItem::ABaseItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

#pragma region /** Networking */
    bReplicates = true;
    SetReplicateMovement(true);
#pragma endregion

#pragma region /** Component Initialization */
    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    }

    // MeshComponent
    if (!MeshComponent)
    {
        MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
        InitMeshComp(MeshComponent);
        MeshComponent->SetupAttachment(RootComponent);
    }

    // ReachComponent
    if (!ReachComponent)
    {
        ReachComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("ReachComponent"));
        InitReachComp(ReachComponent);
        ReachComponent->SetupAttachment(MeshComponent);
    }
#pragma endregion
}

// Called when the game starts or when spawned
void ABaseItem::BeginPlay()
{
	Super::BeginPlay();

    // 최초 1회에 한해 아이콘 초기화; 이후 변경사항 발생 시 직접 Refresh 해주어야함
    // Server_InitializeIcon는 매 BeginPlay마다 호출되지만 내부적으로 최초 1회에 한해 로직 실행
    if (HasAuthority() && bShouldInitializeIcon && !bServer_HasInitializedIcon)
    {
        Server_InitializeIcon();
    }
}

void ABaseItem::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    OnPostInitializeComponents();

    // NOTE: 아래 작업들은 OnPostInitializeComponents 이후에 처리되어야 하는데,
    // OnPostInitializeComponents 과정에서 아래 작업들의 결과에 영향을 줄 만한 값이 변경될 소지가 있기 때문이다.

    // 메시 컴포넌트 초기화 및 등록 완료되었으므로 컴포넌트 크기 조정
    ResizeReachCompToMatchItem();

    // 아이템 데이터, 인벤토리 오브젝트 없을 경우 최초 생성
    if (!IsValid(ItemData))
    {
        GenerateItemData(this);
    }
    if (!IsValid(InvObject))
    {
        GenerateInvObject(this);
        GenerateInvObjDescription();
        VerifyInvObject();
    }

    UPrimitiveComponent* PhysComp = GetPhysComponent();
    if (PhysComp)
    {
        PhysComp->bReplicatePhysicsToAutonomousProxy = true;
    }
}

void ABaseItem::OnPostInitializeComponents()
{
    // NOTE:
    // 실행 시점은 AActor::PostInitializeComponents 직전이 가장 적합하며, 다른 곳에서 처리할 경우
    // 레플리케이션이 제대로 안되어 Translation이 제대로 전달되지 않거나, 혹은 
    // 에셋을 메모리에 올려둔 채 다른 곳에서 에셋에 대한 편집(블루프린트 컴파일 등) 시
    // 엔진이 에셋의 프로퍼티를 Reinitialize하게 되면 엔진단에서 응답없음이 발생할 수 있음.

    // 주의: PostInitializeComponents 단에서 컴포넌트 계층구조의 변화를 유발하는 함수는 서버와 클라 모두에서 실행해주어야 한다
    SetRootToMeshComponent();
}

void ABaseItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void ABaseItem::CacheBeforeDestruction(UObject* NewOuter)
{
    InvObject->Rename(nullptr, NewOuter);
    ItemData->Rename(nullptr, NewOuter);
    // 데이터 저장
    CacheToItemData();
}

// Called every frame
void ABaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseItem::ThrowInDirection(const FVector& Direction)
{
    // TODO
}

void ABaseItem::SetRootToMeshComponent()
{

    // 루트를 메쉬로 변경
    // NOTE:
    // 블루프린트 에디터에서 루트를 메쉬 컴포넌트로 설정한 C++ 클래스를 편집시 Detail 패널에 정보가 표시되지 않아 콘텐츠 제작이 어려움.
    // 따라서 생성자에서는 임의의 Scene Component를 만들어 그 아래에 메쉬 컴포넌트를 붙이고,
    // 게임 로직이 실행되기 전에 루트를 메쉬 컴포넌트로 변경해주는 것으로 루트 기반 트랜스폼 처리가 가능하면서 동시에 에디터 편집이 가능하게끔 구조를 제작함.
    // 단 이때 루트 컴포넌트의 수정은 레플리케이션 이전에 처리해주어야 하는데,
    // 그래야 AttachmentReplication을 통해 올바른 트랜스폼 오프셋과 Relative scale이 전달되어 클라이언트가 제대로 된 정보를 사용할 수 있기 때문임.

    // 이 부분을 처리해줘야 자기 자신에게 Attach하려는 상황을 막을 수 있음 
    // AttachToComponent() 내의 경고 발생 지점에 브레이크포인트 걸고 콜백 타고 가면 확인 가능함
    if (IsValid(MeshComponent->GetAttachParent()))
    {
        MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    }

    // 이걸 해주지 않았을 경우 컴포넌트를 교체해주는 것만으로 Transform 정보가 적용이 되지 않음
    // 이때 Scale 정보는 이미 올바른 값이 적용된 상태이므로 변경하지 않음
    FVector RootLocation = RootComponent->GetComponentLocation();
    FRotator RootRotation = RootComponent->GetComponentRotation();

    RootComponent = nullptr;
    if (!SetRootComponent(MeshComponent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to change the ABaseItem root component to mesh / Auth : %d / Outer: %s"), HasAuthority(), *(MeshComponent->GetOuter()->GetName()));
        return;
    }
    MeshComponent->SetWorldLocation(RootLocation, false, nullptr, ETeleportType::ResetPhysics);
    MeshComponent->SetWorldRotation(RootRotation, false, nullptr, ETeleportType::ResetPhysics);
}

void ABaseItem::InitRootComp(UPrimitiveComponent* Component)
{
    check(Component != nullptr);

    Component->SetIsReplicated(true);
}

void ABaseItem::InitMeshComp(UPrimitiveComponent* Component)
{
    check(Component != nullptr);

    InitRootComp(Component);

    Component->SetComponentTickEnabled(false);
    Component->SetSimulatePhysics(bShouldItemSimulatePhysics);
    Component->SetEnableGravity(bShouldItemSimulateGravity);

    Component->SetCollisionProfileName(TEXT("BaseItemMesh"));
    Component->SetGenerateOverlapEvents(false); // 불필요; 단 소울 아이템과 같이 오버랩이 필요한 경우 함수를 상속받아 따로 설정해주어야 함
    Component->SetCollisionResponseToChannel(ECC_PlayerPawn, ECollisionResponse(DefaultItemCollisionWithPawn));
    Component->SetCollisionResponseToChannel(ECC_BotPawn, ECollisionResponse(DefaultItemCollisionWithPawn));

    TRUtils::OptimizePrimitiveComp(Component, false, false/*라이팅 허용*/);
}

void ABaseItem::InitReachComp(UPrimitiveComponent* Component)
{
    check(Component != nullptr);

    Component->SetComponentTickEnabled(false);
    Component->SetIsReplicated(true);
    Component->SetSimulatePhysics(false);
    Component->SetCollisionProfileName(TEXT("ItemReachComp"));
    Component->SetGenerateOverlapEvents(false); // 불필요; 히트를 사용해 인터랙트함
    Component->SetShouldUpdatePhysicsVolume(false);
    Component->SetCanEverAffectNavigation(false);

    // 충돌 시 호출 함수 맵핑
    if (GetLocalRole() == ROLE_Authority)
    {
        Component->OnComponentHit.AddDynamic(this, &ABaseItem::OnHit);
    }
}

void ABaseItem::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    // TODO
}

void ABaseItem::DisableItemCollision()
{
    if (UPrimitiveComponent* ItemPhysComp = GetPhysComponent())
    {
        ItemPhysComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        // 필요 시 SetCollisionResponseTo 함수로 Fine tuning 가능
        return;
    }
    UE_LOG(LogTemp, Error, TEXT("Item %s has no root component, or the root component is not a derivative of UPrimitiveComponent."), *GetName());
    return;
}

bool ABaseItem::DoesItemBlockWithPawn() const
{
    if (const UPrimitiveComponent* ItemPhysComp = GetPhysComponent())
    {
        return (ItemPhysComp->GetCollisionResponseToChannel(ECC_PlayerPawn) == ECollisionResponse::ECR_Block) && (ItemPhysComp->GetCollisionResponseToChannel(ECC_BotPawn) == ECollisionResponse::ECR_Block);
    }
    UE_LOG(LogTemp, Error, TEXT("Item %s has no root component, or the root component is not a derivative of UPrimitiveComponent."), *GetName());
    return false;
}

void ABaseItem::SetItemCollisionWithPawnToDefault()
{
    return SetItemCollisionWithPawnTo(ECollisionResponse(DefaultItemCollisionWithPawn));
}

void ABaseItem::SetItemCollisionWithPawnTo(ECollisionResponse ColResponse)
{
    if (UPrimitiveComponent* ItemPhysComp = GetPhysComponent())
    {
        ItemPhysComp->SetCollisionResponseToChannel(ECC_PlayerPawn, ColResponse);
        ItemPhysComp->SetCollisionResponseToChannel(ECC_BotPawn, ColResponse);
        return;
    }
    UE_LOG(LogTemp, Error, TEXT("Item %s has no root component, or the root component is not a derivative of UPrimitiveComponent."), *GetName());
    return;
}

void ABaseItem::ResizeReachCompToMatchItem()
{
    FVector Extent = GetEstimatedItemSize();
    Extent.X += ReachCompMargin;
    Extent.Y += ReachCompMargin;
    Extent.Z += ReachCompMargin;
    ReachComponent->SetBoxExtent(Extent, true);
}

bool ABaseItem::IsItemSimulatingGravity() const
{
    if (const UPrimitiveComponent* ItemPhysComp = GetPhysComponent())
    {
        return ItemPhysComp->IsGravityEnabled();
    }
    UE_LOG(LogTemp, Error, TEXT("Item %s has no root component, or the root component is not a derivative of UPrimitiveComponent."), *GetName());
    return false;
}

void ABaseItem::SetItemGravityBackToDefault()
{
    return SetItemGravityTo(bShouldItemSimulateGravity);
}

void ABaseItem::SetItemGravityTo(bool bGravity)
{
    if (UPrimitiveComponent* ItemPhysComp = GetPhysComponent())
    {
        return ItemPhysComp->SetEnableGravity(bGravity);
    }
    UE_LOG(LogTemp, Error, TEXT("Item %s has no root component, or the root component is not a derivative of UPrimitiveComponent."), *GetName());
    return;
}

bool ABaseItem::IsItemSimulatingPhysics() const
{
    if (UPrimitiveComponent* ItemPhysComp = GetPhysComponent())
    {
        return ItemPhysComp->IsSimulatingPhysics();
    }
    UE_LOG(LogTemp, Error, TEXT("Item %s has no root component, or the root component is not a derivative of UPrimitiveComponent."), *GetName());
    return false;
}

void ABaseItem::SetItemPhysicsBackToDefault()
{
    return SetItemPhysicsTo(bShouldItemSimulatePhysics);
}

void ABaseItem::SetItemPhysicsTo(bool bPhysics)
{
    if (UPrimitiveComponent* ItemPhysComp = GetPhysComponent())
    {
        return ItemPhysComp->SetSimulatePhysics(bPhysics);
    }
    UE_LOG(LogTemp, Error, TEXT("Item %s has no root component, or the root component is not a derivative of UPrimitiveComponent."), *GetName());
    return;
}

void ABaseItem::CacheToItemData() const
{
    if (!IsValid(ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("ItemData is invalid, Failed to cache to item data!"));
        return;
    }
    if (!ItemData->CacheItem(this))
    {
        UE_LOG(LogTemp, Error, TEXT("ItemData caching has failed!"));
    }
    return;
}

bool ABaseItem::RestoreFromItemData(UItemData* Data)
{
    if (!IsValid(Data))
    {
        UE_LOG(LogTemp, Error, TEXT("Passed-in data is null, Failed to cache to item data!"));
        return false;
    }
    
    this->bServer_HasInitializedIcon = Data->GetCachedHasInitIcon();
    // 필요 시 추가 로직은 개별 클래스에서 구현한다
    return true;
}

void ABaseItem::GenerateItemData(UObject* Outer)
{
    UClass* DataClass = ItemDataClass;
    UItemData* GeneratedData = nullptr;

    if (!IsValid(DataClass))
    {
        UE_LOG(LogTemp, Error, TEXT("Item %s has no default ItemDataClass set."), *GetName());
    }
    else
    {
        ItemData = NewObject<UItemData>(Outer, ItemDataClass, TEXT("ItemData"));
    }
    return;
}

void ABaseItem::GenerateInvObject(UObject* Outer)
{
    UClass* TypeClass = InvObjectClass;
    UInvObject* GeneratedObj = nullptr;

    if (!IsValid(TypeClass))
    {
        UE_LOG(LogTemp, Error, TEXT("Item %s has no default InvObjectClass set."), *GetName());
    }
    else
    {
        GeneratedObj = NewObject<UInvObject>(Outer, TypeClass, TEXT("InvObject"));
        GeneratedObj->SetItemData(ItemData);
        GeneratedObj->SetBaseItemClass(GetClass()); // GetClass는 BP일 경우 BP 클래스를 반환한다
        InvObject = GeneratedObj;
    }
    return;
}

void ABaseItem::VerifyInvObject()
{
    if (!IsValid(InvObject))
    {
        UE_LOG(LogTemp, Warning, TEXT("Item %s has no InvObject Set. Using default object. This could result in an unintended behaviour."), *this->GetName());
        InvObject = CreateDefaultInvObject();
    }
}

UInvObject* ABaseItem::CreateDefaultInvObject()
{
    return NewObject<UInvObject>();
}

bool ABaseItem::OnItemEquip(UEquipSystem* EquSys, int32 SlotIdx)
{
    // ABaseItem은 장착이 불가능
    // NOTE: 게임플레이 시스템 상 장착은 불가하지만 소켓에 물리적으로 부착하는 것은 지원함
    FString DebugString = FString::Printf(TEXT("Item %s is not a AWieldItem. You cannot Equip it."), *this->GetName());
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, DebugString);
    return false;
}

FVector ABaseItem::GetEstimatedItemSize()
{
    if (!GetMeshComponent()) return FVector(0, 0, 0);
    return GetMeshComponent()->GetLocalBounds().BoxExtent;
}

void ABaseItem::RegisterVisibility(bool bVisibility)
{
    // BaseItem은 동적 메쉬가 없으므로 그냥 바로 처리한다
    SetItemVisibility(bVisibility);
}

void ABaseItem::SetItemVisibility(bool bVisibility)
{
    bShouldRenewItemVisibility = false; // 갱신했으니 false로 처리
    if (GetMeshComponent())
    {
        GetMeshComponent()->SetVisibility(bVisibility, true);
    }
    bItemVisibility = bVisibility;
}

void ABaseItem::PrintItemDataTestValue()
{
    FString DebugString = FString::Printf(TEXT("ItemData TestVal: %f"), ItemData->TestVal);
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, DebugString);
    ItemData->TestVal += 10;
}

bool ABaseItem::OnItemPickup(UInventoryComponent* InvComp)
{
    if (IsValid(InvComp))
    {
        if (InvComp->TryAddInvObject(this->InvObject))
        {
            if (HasAuthority())
            {
                CacheBeforeDestruction(InvComp->GetOwner());
                Destroy();
            }
            return true;
        }
    }
    return false;
}

void ABaseItem::SetInvObject(UInvObject* InvObj)
{
    InvObject = InvObj;
}

TObjectPtr<class UInvObject> ABaseItem::GetInvObject()
{
    return InvObject;
}

void ABaseItem::SetItemData(UItemData* Data)
{
    ItemData = Data;
}

void ABaseItem::GenerateInvObjDescription()
{
    if (!InvObject)
    {
        UE_LOG(LogTemp, Error, TEXT("GenerateInvObjDescription - InvObject is null! Item: %s"), *(GetName()));
        return;
    }
    InvObject->SetInvObjDesc(TempItemDesc);
    InvObject->SetInvObjAttr(TempItemAttributesForUI);
}

void ABaseItem::Server_InitializeIcon()
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Server_InitializeIcon - Client should not call this function!"));
        return;
    }

    UInvObject* InvObj = GetInvObject();
    if (HasAuthority() && InvObj)
    {
        // 캐싱을 처리해야 메쉬 정보가 데이터에 기록되어 아이콘 생성 과정에서 사용할 수 있음
        bServer_HasInitializedIcon = true; // 아이콘 초기화가 완료되었음 또한 캐싱되어야 하므로 CacheToItemData보다 먼저 호출되어야 함
        CacheToItemData(); // 캐싱
        GetInvObject()->Server_RequestUpdateIcon();
        return;
    }
    UE_LOG(LogTemp, Error, TEXT("Server_InitializeIcon - Something went wrong!"));
    return;
}

