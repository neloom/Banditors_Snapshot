// Copyright (C) 2024 by Haguk Kim


#include "GameCharacter.h"
#include "Components/CapsuleComponent.h"
#include "AnimConfig.h"
#include "InventoryComponent.h"
#include "EquipSystem.h"
#include "SlotComponent.h"
#include "WieldItem.h"
#include "MuzzleTriggeredActor.h"
#include "GunItem.h"
#include "GunItemData.h"
#include "ActItemComponent.h"
#include "TRPlayerState.h"
#include "Components/BoxComponent.h"
#include "HitboxComponent.h"
#include "OuterHitboxComponent.h"
#include "RecoilAnimationComponent.h"
#include "TRRoomObserverComponent.h"
#include "TRHUDWidget.h"
#include "RoomLevel.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectTRGameModeBase.h"
#include "TimerManager.h"
#include "TRDamageType.h"
#include "DamageTypeNeutral.h"
#include "DamageTypePhysical.h"
#include "DamageTypeElemental.h"
#include "DamageTypeMagical.h"
#include "Engine/DamageEvents.h"

// 게임 내 모든 캐릭터에 대해 CharacterMovementComponent 재정의
AGameCharacter::AGameCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;

#pragma region /** Component Initialization */
	// 캡슐
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp != nullptr);
	CapsuleComp->SetCollisionProfileName("PlayerCapsule");
	CapsuleComp->SetGenerateOverlapEvents(true); // 각종 이벤트 판정에 사용
	CapsuleComp->SetShouldUpdatePhysicsVolume(false);

	// 메쉬
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	check(SkeletalMesh != nullptr);

	SkeletalMesh->SetSimulatePhysics(false);
	SkeletalMesh->SetEnableGravity(false);
	SkeletalMesh->SetCollisionProfileName("CharSkelMesh");
	SkeletalMesh->SetGenerateOverlapEvents(false); // 불필요
	SkeletalMesh->SetShouldUpdatePhysicsVolume(false);

	// 반동
	RecoilComponent = CreateDefaultSubobject<URecoilAnimationComponent>(TEXT("RecoilComponent"));
	RecoilComponent->SetIsReplicated(true);
	RecoilComponent->SetFireMode(EFireMode_PRAS::Semi); // 동적으로 변경된다

	// 룸 옵저버
	RoomObserverComp = CreateDefaultSubobject<UTRRoomObserverComponent>(TEXT("RoomObserverComponent"));
	RoomObserverComp->bAutoActivate = false; // 필요 시 수동으로 가동한다
	check(RoomObserverComp != nullptr);
	RoomObserverComp->Deactivate();

	// 외곽 히트박스
	OuterHitbox = CreateDefaultSubobject<UOuterHitboxComponent>(TEXT("CharOuterBox"));
	check(OuterHitbox != nullptr);
	OuterHitbox->SetupAttachment(RootComponent);

	// 디테일한 충돌 판정처리용 박스 컴포넌트
	HeadColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("HeadBox"));
	TorsoColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("TorsoBox"));
	PelvisColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("PelvisBox"));
	RArmUpperColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("RArmUpperBox"));
	RArmLowerColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("RArmLowerBox"));
	RHandColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("RHandBox"));
	LArmUpperColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("LArmUpperBox"));
	LArmLowerColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("LArmLowerBox"));
	LHandColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("LHandBox"));
	RLegUpperColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("RLegUpperBox"));
	RLegLowerColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("RLegLowerBox"));
	RFootColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("RFootBox"));
	LLegUpperColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("LLegUpperBox"));
	LLegLowerColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("LLegLowerBox"));
	LFootColComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("LFootBox"));

	// 그룹 초기화
	DetailColComponents = {
		HeadColComponent,
		TorsoColComponent,
		PelvisColComponent,
		RArmUpperColComponent,
		RArmLowerColComponent,
		RHandColComponent,
		LArmUpperColComponent,
		LArmLowerColComponent,
		LHandColComponent,
		RLegUpperColComponent,
		RLegLowerColComponent,
		RFootColComponent,
		LLegUpperColComponent,
		LLegLowerColComponent,
		LFootColComponent
	};

	// 정보 초기화
	InitHitbox();

	// 네비게이션 충돌 해제
	SetCanAffectNavigation(false);

	// 인벤토리
	InvComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	check(InvComponent != nullptr);

	// 장착품
	EquipSystem = CreateDefaultSubobject<UEquipSystem>(TEXT("Equipments"));
	check(EquipSystem != nullptr);
}

// Called when the game starts or when spawned
void AGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 룸 옵저버를 사용하는 캐릭터일 경우 가동한다
	if (bShouldActivateRoomObserver)
	{
		RoomObserverComp->Activate(false);
	}
	else
	{
		// 단순 Deactivation만으로는 컴포넌트가 완전히 동작을 정지하지 않음
		RoomObserverComp->DestroyComponent();
	}

	if (!HasAuthority())
	{
		// 클라이언트의 경우 시야 밖의 애니메이션 렌더링은 어떠한 경우에도 불필요하다
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	}

	// 최초 1회 히트박스 상대 트랜스폼 캐싱
	CacheHitboxDeltaRelativeTransform();

	// 기본적으로 세부 히트박스는 비활성화한다
	DeactivateDetailedHitbox();

	// 상대 트랜스폼 캐싱이 끝났으므로 더이상 추적할 필요 없음; 전체 탈착
	for (UHitboxComponent* DetailedHitbox : DetailColComponents)
	{
		DetailedHitbox->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AGameCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ExecRemainingDeferredTask();
	Super::EndPlay(EndPlayReason);
}

void AGameCharacter::SetCanAffectNavigation(bool Value)
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (IsValid(SkeletalMesh))
	{
		SkeletalMesh->SetCanEverAffectNavigation(Value);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (IsValid(CapsuleComp))
	{
		CapsuleComp->SetCanEverAffectNavigation(Value);
	}
	
	for (UActorComponent* BodyColComp : DetailColComponents)
	{
		if (IsValid(BodyColComp))
		{
			BodyColComp->SetCanEverAffectNavigation(Value);
		}
	}
}

void AGameCharacter::DestroySelf()
{
	Destroy();
}

void AGameCharacter::ExecRemainingDeferredTask()
{
	if (HasAuthority())
	{
		// 큐에 남은 작업들을 모두 한번에 강제로 처리한다
		if (!Server_DropDeferredItemQueue.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("AGameCharacter::EndPlay - GameCharacter %s has remaining drop queue task. Consider extending the actor lifetime."), *(GetName()));
			Server_ExecuteDropForDeferredItem(INT32_MAX);
		}
		if (!Server_SpawnDeferredItemQueue.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("AGameCharacter::EndPlay - GameCharacter %s has remaining spawn queue task. Consider extending the actor lifetime."), *(GetName()));
			Server_ExecuteSpawnForDeferredItem(INT32_MAX);
		}
	}
}

FGameCharacterInstanceData AGameCharacter::Server_GetInstanceData()
{
	// 필요 시 개별 클래스에서 오버라이드 해 사용한다
	FGameCharacterInstanceData NewData;

	NewData.bPureMaxHealthCached = true;
	NewData.PureMaxHealth = PureMaxHealth;

	NewData.bHealthPointCached = true;
	NewData.HealthPoint = HealthPoint;

	NewData.bHasDiedCached = true;
	NewData.bHasDied = bHasDied;

	if (InvComponent)
	{
		InvComponent->Server_ExportInventoryInfo(&NewData.InvObjectValues, &NewData.InvObjectDatas);
		NewData.bInventoryCached = true;
	}

	if (EquipSystem)
	{
		// 현재 Deploy중인 무기가 있을 경우 현재 정보를 캐싱한다
		AWieldItem* DeployedItem = EquipSystem->GetCurrWeaponActor();
		if (DeployedItem)
		{
			DeployedItem->CacheBeforeDestruction(nullptr /* 아우터 정보는 변경하지 않는다 */);
		}

		EquipSystem->Server_ExportInventoryInfo(&NewData.EquObjectValues, &NewData.EquObjectDatas);
		NewData.CurrentSlot = EquipSystem->GetCurrentSlot();
		NewData.bEquipSysCached = true;
	}
	return NewData;
}

bool AGameCharacter::Server_RestoreFromInstanceData(const FGameCharacterInstanceData& InstData)
{
	// 필요 시 개별 클래스에서 오버라이드 해 사용한다
	if (InstData.bPureMaxHealthCached)
	{
		this->SetPureMaxHealthTo(InstData.PureMaxHealth);
	}

	if (InstData.bHealthPointCached)
	{
		this->SetHealthPointTo(InstData.HealthPoint);
	}

	if (InstData.bHasDiedCached)
	{
		this->SetHasDiedTo(InstData.bHasDied);
	}

	if (InvComponent && InstData.bInventoryCached)
	{
		if (InstData.InvObjectDatas.Num() != InstData.InvObjectValues.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("Server_RestoreFromInstanceData - Invalid inventory instance data!"));
			return false;
		}
		InvComponent->Server_ImportInvComp(InstData.InvObjectValues, InstData.InvObjectDatas);
	}

	if (EquipSystem && InstData.bEquipSysCached)
	{
		if (InstData.EquObjectDatas.Num() != InstData.EquObjectValues.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("Server_RestoreFromInstanceData - Invalid equip system instance data!"));
			return false;
		}
		EquipSystem->Server_ImportInvComp(InstData.EquObjectValues, InstData.EquObjectDatas);
		// NOTE: Switch 상태는 유지하지 않는다; 해당 로직은 서버-클라 모두 호출되어야 하고, 게임플레이에도 큰 영향이 없기 때문
	}
	return true;
}

void AGameCharacter::Server_RegisterDestructionAfter(float TimeLeft)
{
	if (!HasAuthority()) return;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(DestructionTimerHandle, this, &AGameCharacter::DestroySelf, TimeLeft, false);
	}
}

void AGameCharacter::Server_OnDamageInflictedToTarget(AGameCharacter* Target, FDamageEvent const& DamageEvent, int32 Damage, bool bIsKillshot, bool bIsCrit)
{
	// GameCharacter는 기본적으로 아무것도 처리하지 않는다
	return;
}

UHitboxComponent* AGameCharacter::GetNearestDetailedHitboxFrom(const FVector& WorldLocation)
{
	UHitboxComponent* Found = nullptr;
	float Distance = MAX_FLT;
	for (UHitboxComponent* Hitbox : DetailColComponents)
	{
		if (Hitbox)
		{
			float CurrDist = FVector::Dist(WorldLocation, Hitbox->GetComponentLocation());
			if (CurrDist < Distance)
			{
				Distance = CurrDist;
				Found = Hitbox;
			}
		}
	}
	return Found;
}

void AGameCharacter::Server_RegisterFireRPC_Implementation(bool bIsPrimary)
{
	if (HasAuthority())
	{
		if (EquipSystem)
		{
			AWieldItem* CurrWeaponActor = EquipSystem->GetCurrWeaponActor();
			if (IsValid(CurrWeaponActor))
			{
				// 컴포넌트 트리거
				if (bIsPrimary && IsValid(CurrWeaponActor->PrimaryActComponent)) 
				{
					CurrWeaponActor->PrimaryActComponent->Trigger(this);
				}
				else if (!bIsPrimary && IsValid(CurrWeaponActor->SecondaryActComponent)) 
				{
					CurrWeaponActor->SecondaryActComponent->Trigger(this);
				}
			}
		}
	}
	return;
}

void AGameCharacter::Server_RegisterStopFireRPC_Implementation(bool bIsPrimary)
{
	if (HasAuthority())
	{
		if (EquipSystem)
		{
			AWieldItem* CurrWeaponActor = EquipSystem->GetCurrWeaponActor();
			if (IsValid(CurrWeaponActor))
			{
				// 컴포넌트 트리거
				if (bIsPrimary && IsValid(CurrWeaponActor->PrimaryActComponent))
				{
					CurrWeaponActor->PrimaryActComponent->Stop(this);
				}
				else if (!bIsPrimary && IsValid(CurrWeaponActor->SecondaryActComponent))
				{
					CurrWeaponActor->SecondaryActComponent->Stop(this);
				}
			}
		}
	}
	return;
}

void AGameCharacter::Server_RegisterSwitchToRPC_Implementation(int32 SlotIdx)
{
	if (HasAuthority())
	{
		// 서버 로직 (Actual Deployment)
		if (EquipSystem)
		{
			EquipSystem->Server_SwitchWeaponSlotTo(SlotIdx);
		}
	}
	return;
}

void AGameCharacter::Server_ProcessPickup(ABaseItem* TargetItem)
{
	if (HasAuthority())
	{
		if (TargetItem)
		{
			ProcessBeforeItemPickup(TargetItem);
			if (InvComponent)
			{
				if (TargetItem->OnItemPickup(InvComponent))
				{
					// TODO: 픽업 성공 시 별도 로직 필요 시 추가
				}
			}
		}
	}
	return;
}

void AGameCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	GetTRCharacterMovementComponent()->UpdateLandingState();
}

TArray<FHitResult> AGameCharacter::Reach()
{
	return TArray<FHitResult>();
}

ABaseItem* AGameCharacter::ReachItem()
{
	return nullptr;
}

void AGameCharacter::ProcessBeforeItemPickup(ABaseItem* ReachedItem)
{
	// TODO: 로직 필요 시 추가
	// NOTE: FPSCharacter 로직 참고
}

void AGameCharacter::Server_ProcessMuzzleTrigger(AMuzzleTriggeredActor* TargetActor)
{
	if (TargetActor)
	{
		TargetActor->MuzzleTrigger(this);
	}
}

void AGameCharacter::Server_AddDeferredDropItem(UInvObject* InvObj, FVector Location)
{
	if (!HasAuthority()) return;
	Server_DropDeferredItemQueue.Enqueue(TPair<UInvObject*, FVector>(InvObj, Location));
}

void AGameCharacter::Server_AddDeferredSpawnItem(TSubclassOf<ABaseItem> ItemClass, FVector Location)
{
	if (!HasAuthority()) return;
	Server_SpawnDeferredItemQueue.Enqueue(TPair<TSubclassOf<ABaseItem>, FVector>(ItemClass, Location));
}

void AGameCharacter::Server_ExecuteDropForDeferredItem(int32 MaxCount)
{
	if (!HasAuthority()) return;
	if (!InvComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_ExecuteDropForDeferredItem - GameCharacter has no InvComp!"));
		return;
	}

	for (int32 Count = 0; Count < MaxCount; ++Count)
	{
		if (Server_DropDeferredItemQueue.IsEmpty()) break;
		TPair<UInvObject*, FVector> CurrObj;
		Server_DropDeferredItemQueue.Dequeue(CurrObj);
		
		UInvObject* InvObj = CurrObj.Get<0>();
		const FVector& DropLoc = CurrObj.Get<1>();

		if (InvObj)
		{
			// 이미 인벤토리에서는 제거되었다고 가정하므로 별도의 추가 로직 없음
			InvComponent->Server_SpawnItemFromInvObjectRPC(InvObj, DropLoc);
		}
	}
}

void AGameCharacter::Server_ExecuteSpawnForDeferredItem(int32 MaxCount)
{
	if (!HasAuthority()) return;
	UWorld* World = GetWorld();
	if (!World) return;
	AProjectTRGameModeBase* TRGM = World->GetAuthGameMode<AProjectTRGameModeBase>();
	if (!TRGM) return;

	for (int32 Count = 0; Count < MaxCount; ++Count)
	{
		if (Server_SpawnDeferredItemQueue.IsEmpty()) break;
		TPair<TSubclassOf<ABaseItem>, FVector> CurrObj;
		Server_SpawnDeferredItemQueue.Dequeue(CurrObj);

		TSubclassOf<ABaseItem> SpawnClass = CurrObj.Get<0>();
		const FVector& SpawnLoc = CurrObj.Get<1>();

		if (SpawnClass)
		{
			ABaseItem* DroppedItem = TRGM->SpawnItem(SpawnClass, World, SpawnLoc, FRotator(), FActorSpawnParameters());
		}
	}
}

void AGameCharacter::DeferredTaskTick(float DeltaTime)
{
	if (!HasAuthority()) return;
	CurrDeferredTickTime += DeltaTime;
	if (CurrDeferredTickTime < DeferredTaskTickCycle)
	{
		return;
	}
	CurrDeferredTickTime = 0.0f;

	if (!Server_DropDeferredItemQueue.IsEmpty())
	{
		Server_ExecuteDropForDeferredItem(1);
	}
	if (!Server_SpawnDeferredItemQueue.IsEmpty())
	{
		Server_ExecuteSpawnForDeferredItem(1);
	}
}

void AGameCharacter::SwitchWeaponSlotTo(int32 SlotIdx)
{
	// NOTE: 클라 로직 필요 시 여기가 아닌 Multicast 함수에 작성할 것
	Server_RegisterSwitchToRPC(SlotIdx);
}

void AGameCharacter::Server_RefillAllAmmo()
{
	// 장착 총기
	if (EquipSystem)
	{
		const TArray<UInvObject*>& EquObjects = EquipSystem->GetAllInvObjects();
		for (UInvObject* EquObj : EquObjects)
		{
			if (EquObj->GetBaseItemClass() && EquObj->GetBaseItemClass()->IsChildOf(AGunItem::StaticClass()))
			{
				// ItemData의 탄약을 최대치만큼 채운다
				UGunItemData* GunData = Cast<UGunItemData>(EquObj->GetItemData());
				if (GunData)
				{
					GunData->CacheCurrAmmo(-1 /* 탄약을 최대치로 채운다 */);
				}

				// 액터가 생성되어 있는 경우 아이템에도 즉각 변경사항을 적용한다 (Deploy중인 경우)
				AGunItem* GunOwner = Cast<AGunItem>(EquipSystem->GetCurrWeaponActor());
				if (GunOwner)
				{
					GunOwner->CurrAmmo = GunOwner->GetStat_MaxAmmo(this);
				}
			}
		}
	}
}

void AGameCharacter::AttachItem(ABaseItem* AttachItem)
{
	FName SocketName = GetSockNameOfPart(AttachItem->GetCharacterAttachPartName());
	FVector RelativeLocation = AttachItem->GetAttachRelativeLocation();
	FRotator RelativeRotation = AttachItem->GetAttachRelativeRotation();

	if (!GetMesh()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to attach item to socket: %s. Mesh is invalid. / Auth: %d"), *SocketName.ToString(), HasAuthority());
		return;
	}

	if (!IsValid(AttachItem))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to attach item to socket: %s. Item pointer is invalid. / Auth: %d"), *SocketName.ToString(), HasAuthority());
		return;
	}

	if (AttachItem->AttachToComponent(GetMesh(), AttachItem->GetEquipAttachRule(), SocketName))
	{
		if (HasAuthority())
		{
			AttachItem->SetActorRelativeLocation(RelativeLocation);
			AttachItem->SetActorRelativeRotation(RelativeRotation);
		}
		UE_LOG(LogTemp, Log, TEXT("Attached item to socket: %s / Auth: %d"), *SocketName.ToString(), HasAuthority());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to attach item to socket: %s. This can happen if you are attaching a physics object to another physics object. To avoid this, you can toggle off one of the object's bSimaltePhysics. Read SceneComponent.cpp. / Auth: %d"), *SocketName.ToString(), HasAuthority());
	return;
}

void AGameCharacter::DetachItem(ABaseItem* AttachedItem)
{
	if (GetMesh())
	{
		const TArray<USceneComponent*>& AttachedComponents = GetMesh()->GetAttachChildren();
		for (USceneComponent* Component : AttachedComponents)
		{
			if (Component == AttachedItem->GetRootComponent())
			{
				AttachedItem->DetachFromActor(AttachedItem->GetEquipDetachRule());
				UE_LOG(LogTemp, Log, TEXT("Detached item: %s / Auth: %d"), *AttachedItem->GetName(), HasAuthority());
				return;
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Failed to detach item from socket"));
	return;
}

UStatusEffect* AGameCharacter::Server_GenerateAndAddStatEffect(const FStatEffectGenInfo& Data, AGameCharacter* EffectApplier)
{
	if (!HasAuthority()) return nullptr;
	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Error, TEXT("Server_GenerateAndAddStatEffect - This character is invalid! Aborting."));
		return nullptr;
	}

	if (Data.StatusEffectId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_GenerateAndAddStatEffect - FStatEffectGenInfo.StatusEffectId is empty. You should assign it a proper id."));
	}
	if (Data.StatusName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_GenerateAndAddStatEffect - FStatEffectGenInfo.StatusName is empty."));
	}

	// 기존에 동일한 id를 가지는 인스턴스가 존재하는지 확인한다
	// NOTE: EffectApplier가 다르더라도 id만 같으면 같다고 취급한다
	// 즉 몬스터가 플레이어 A에 의해 화염 상태이상을 입었고, 그 다음 플레이어 B에 의해 다시 한번 화염 상태이상을 입으면
	// 해당 상태이상의 지속시간은 변경되더라도, EffectApplier는 A로 유지된다
	UStatusEffect* PreExistingInst = Server_FindStatEffectOfId(Data.StatusEffectId);
	if (PreExistingInst != nullptr)
	{
		if (!Data.bForceNewInstance)
		{
			// 새 인스턴스 생성할 필요가 없는 경우 타이머를 주어진 방식에 맞게 수정
			PreExistingInst->Server_SetDuration(Data.StatDuration, Data.TimerHandleMethod);

			// NOTE: 지속시간만 수정된 것이기 때문에 Server_OnStatEffectChanged는 불필요
			// NOTE: 만약 PreExistingInst에 대한 액세스를 얻으려면 Server_FindStatEffectOfId를 사용할 것

			// 새 인스턴스가 생성된 것이 아니므로 nullptr 반환
			return nullptr;
		}
	}

	// 새 인스턴스 생성 후 추가
	UStatusEffect* StatEffect = NewObject<UStatusEffect>(this);
	if (StatEffect)
	{
		// 이후 무언가 값이 변경된다면 레플리케이션 되야 하므로 가장 먼저 처리할 것
		AddReplicatedSubObject(StatEffect);

		StatEffect->SetStatusEffectId(Data.StatusEffectId);
		StatEffect->SetStatName(Data.StatusName);
		StatEffect->SetStatDesc(Data.StatusDesc);
		StatEffect->SetEffectDamage(Data.DamageInfo);
		StatEffect->SetStatDelta(Data.StatModifier);

		AppliedStatEffects.Add(StatEffect);
		StatEffect->Server_OnSelfAddedToParent(this, EffectApplier);

		// Server_OnSelfAddedToParent 이후 처리할 것
		StatEffect->Server_SetDuration(Data.StatDuration, EStatTimerHandleMethod::STHM_ForceOverride);

		// 가장 마지막에 처리
		StatEffect->Server_ValidateSelf(Data.StatDuration);
		Server_OnStatEffectChanged();
	}
	return StatEffect;
}

void AGameCharacter::Server_RemoveStatEffect(UStatusEffect* StatEffect)
{
	// NOTE: nullptr를 입력으로 받을 수 있다
	if (!HasAuthority() || !StatEffect) return;
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(StatEffect->Server_DurationTimer);
	}

	RemoveReplicatedSubObject(StatEffect);
	AppliedStatEffects.Remove(StatEffect);

	Server_OnStatEffectChanged();
}

UStatusEffect* AGameCharacter::Server_FindStatEffectOfId(const FString& Id)
{
	for (UStatusEffect* Effect : AppliedStatEffects)
	{
		if (Effect->GetStatusEffectId().Equals(Id))
		{
			return Effect;
		}
	}
	return nullptr;
}

void AGameCharacter::Server_ClearAllStatEffect()
{
	if (!HasAuthority()) return;
	TArray<UStatusEffect*> RemoveEffects = AppliedStatEffects;
	for (UStatusEffect* RemoveEffect : RemoveEffects)
	{
		Server_RemoveStatEffect(RemoveEffect);
	}
}

void AGameCharacter::Server_OnStatEffectChanged()
{
	if (!HasAuthority()) return;

	Server_ResetDeltaStatusSumCached();
	for (const UStatusEffect* Effect : AppliedStatEffects)
	{
		const FCharacterStatModifier& StatDelta = Effect->GetStatDelta();
		Server_AddStatusDeltaToSum(StatDelta);
	}

	// 캐싱해야 하는 값들을 전부 새로 캐싱한다
	RealMaxHealthCached = GetStat_MaxHealth();

	UBaseCharacterMovementComponent* TRCM = GetTRCharacterMovementComponent();
	if (TRCM)
	{
		TRCM->SetDeltaMaxWalkSpeedCached(StatusDeltaSumCached.DeltaMaxWalkSpeed);
		TRCM->SetDeltaJumpCountCached(StatusDeltaSumCached.DeltaJumpCount);
		TRCM->SetDeltaJumpSpeedCached(StatusDeltaSumCached.DeltaJumpSpeed);
		TRCM->SetDeltaRollSpeedCached(StatusDeltaSumCached.DeltaRollSpeed);
		TRCM->SetDeltaRollDelayCached(StatusDeltaSumCached.DeltaRollDelay);
	}

	// 무기에 스테이터스를 적용한다
	if (EquipSystem && EquipSystem->GetCurrWeaponActor())
	{
		EquipSystem->GetCurrWeaponActor()->OnDeployerStatChanged();
	}
}

void AGameCharacter::Server_ResetDeltaStatusSumCached()
{
	if (!HasAuthority()) return;

	// 전체 초기화
	StatusDeltaSumCached = FCharacterStatModifier();
}

void AGameCharacter::Server_AddStatusDeltaToSum(const FCharacterStatModifier& StatModifier)
{
	if (!HasAuthority()) return;

	StatusDeltaSumCached.AddToThis(StatModifier);
}

void AGameCharacter::OnRep_UpdateHealth()
{
	OnHealthUpdate();
}

void AGameCharacter::OnHealthUpdate()
{
	if (HasAuthority())
	{
		FString Message = FString::Printf(TEXT("%s - %d hp"), *GetFName().ToString(), HealthPoint);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, Message);

		if (HealthPoint <= 0)
		{
			HealthPoint = 0;
			Server_OnHealthReachedZeroOrLower();
			UE_LOG(LogTemp, Warning, TEXT("Server_OnHealthReachedZeroOrLower"));
		}
	}

	// UI 업데이트 (서버,클라)
	if (Local_BoundHUDWidget.IsValid())
	{
		Local_BoundHUDWidget->UpdateHealthBar();
	}
}

void AGameCharacter::Server_OnHealthReachedZeroOrLower()
{
	SetHasDiedTo(true);
}

float AGameCharacter::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("AGameCharacter::TakeDamage - Client should not handle damage calculation!"));
		return 0.0f;
	}

	// 데미지 타입 지정이 없을 경우 Neutral 타입 사용
	TSubclassOf<UTRDamageType> DamageType = nullptr;
	if (!DamageEvent.DamageTypeClass || !DamageEvent.DamageTypeClass->IsChildOf(UTRDamageType::StaticClass()))
	{
		DamageType = UDamageTypeNeutral::StaticClass();
	}
	else
	{
		DamageType = DamageEvent.DamageTypeClass;
	}

	// 타입별 데미지 증감 적용
	float ModifiedDamage = Server_ModifyDamageByType(DamageTaken, DamageType);

	// 데미지 정수로 ceil
	int32 ActualDamage = FMath::CeilToInt32<float>(ModifiedDamage);

	// 아군이 아닌 게임 캐릭터에 의한 데미지는 기록
	AGameCharacter* NonAllyAttacker = nullptr;
	AGameCharacter* DamageChar = Cast<AGameCharacter>(DamageCauser);
	if (DamageChar && !TRUtils::IsAllyWith(DamageChar, this))
	{
		NonAllyAttacker = DamageChar;
	}

	// 동맹이 아니며, 데미지가 0 이상일 경우만 공격으로 취급
	if (IsValid(NonAllyAttacker) && ActualDamage >= 0)
	{
		Server_SetLastAttackedBy(NonAllyAttacker);
	}
	
	// 실연산 처리
	// NOTE: 반드시 SetLastAttackedBy 로직을 처리한 이후 처리할 것
	bool bWasAliveBeforeDamage = !this->GetHasDied();
	SetHealthPointTo(HealthPoint - ActualDamage);
	bool bIsKilledByDamage = (bWasAliveBeforeDamage && this->GetHasDied());

	// 데미지 값이 0 초과일때 공격자 콜백 (적대 관계 무관)
	if (IsValid(DamageChar) && ActualDamage > 0)
	{
		DamageChar->Server_OnDamageInflictedToTarget(this, DamageEvent, ActualDamage, bIsKilledByDamage, false/* TODO: DamageType 기반 크리티컬 판정 */);
	}
	return ActualDamage;
}

int32 AGameCharacter::GetStat_MaxHealth() const
{
	if (GetNetMode() == NM_Client)
	{
		return RealMaxHealthCached;
	}
	else
	{
		return PureMaxHealth + StatusDeltaSumCached.DeltaMaxHealth;
	}
}

void AGameCharacter::SetPureMaxHealthTo(const int32 Value)
{
	if (HasAuthority())
	{
		PureMaxHealth = Value;
		RealMaxHealthCached = GetStat_MaxHealth(); // 클라에게 새 변경값을 전달하기 위해 캐싱해야 한다
		SetHealthPointTo(HealthPoint); // 변경된 범위에 맞게 재업데이트
	}
}

void AGameCharacter::SetHealthPointTo(const int32 Value)
{
	if (HasAuthority())
	{
		HealthPoint = FMath::Clamp<int32>(Value, 0, GetStat_MaxHealth());
		OnHealthUpdate();
	}
}

void AGameCharacter::OnRep_UpdateLife()
{
	OnLifeUpdate();
}

void AGameCharacter::OnLifeUpdate()
{
	if (HasAuthority())
	{
		FString Message = FString::Printf(TEXT("%s - life %d."), *GetFName().ToString(), bHasDied);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, Message);

		if (bHasDied)
		{
			Server_OnDeath();
		}
	}
}

void AGameCharacter::Server_OnDeath()
{
	Server_OnDeathDelegate.Broadcast(this);

	Server_ProcessDeath();

	// Destroy는 반드시 ProcessDeath 이후에 호출되어야 함
	Server_RegisterDestructionAfter(DestructionTimeDefault);

	Multicast_OnDeathRPC();
}

void AGameCharacter::Multicast_OnDeathRPC_Implementation()
{
	Multicast_ProcessDeath();
}

void AGameCharacter::Server_ProcessDeath()
{
	// 마지막 공격자 대상 로직 처리
	AGameCharacter* LastAttacker = Server_GetLastAttackedByAndDeref();
	if (IsValid(LastAttacker))
	{
		Server_OnDeathHandleLastAttacker(LastAttacker);
	}
	
	// 소지품 드랍 (Deferred)
	Server_DropAllItemFromInv(OnDeathDropOffset);
	Server_DropAllEquippedItems(OnDeathDropOffset);

	// 상태이상 변화
	// NOTE: 소지품 드랍과 같이 StatEffect에 영향을 줄 수 있는 모든 과정을 마무리한 이후 처리
	Server_ClearAllStatEffect();

	// 래그돌화
	Server_Ragdollfy();
	return;
}

void AGameCharacter::Multicast_ProcessDeath()
{
	return;
}

void AGameCharacter::SetHasDiedTo(const bool Value)
{
	if (HasAuthority())
	{
		if (bHasDied != Value)
		{
			bHasDied = Value;
			OnLifeUpdate();
		}
	}
}

AGameCharacter* AGameCharacter::Server_GetLastAttackedByAndDeref()
{
	return Server_LastAttakedBy;
}

void AGameCharacter::Server_SetLastAttackedBy(AGameCharacter* Character)
{
	if (!IsValid(Character))
	{
		return;
	}
	Server_LastAttakedBy = Character;
}

void AGameCharacter::Server_OnDeathHandleLastAttacker(AGameCharacter* Attacker)
{
	// 공격자 추가 효과 필요 시 적용
	if (Attacker && Attacker->EquipSystem)
	{
		// 무기 효과 적용
		// NOTE: 격발 시점이 아닌, 사망 시점에 들고 있는 무기로 판정한다
		AGunItem* GunItem = Cast<AGunItem>(Attacker->EquipSystem->GetCurrWeaponActor());
		if (GunItem)
		{
			// 처치 시 효과 부여
			UGameplayStatics::ApplyDamage(
				Attacker,
				GunItem->GetStat_SelfDmgOnCharacterKilled(Attacker),
				Attacker->Controller,
				Attacker,
				UDamageTypeNeutral::StaticClass()
			);
		}
	}

	return;
}

float AGameCharacter::Server_ModifyDamageByType(float OriginDamage, TSubclassOf<class UTRDamageType> DamageType) const
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_ModifyDamageByType - Client should not call this function!"));
		return OriginDamage;
	}

	if (DamageType->IsChildOf(UDamageTypeNeutral::StaticClass()))
	{
		// neutral은 데미지 그대로 받음
		return OriginDamage;
	}
	else if (DamageType->IsChildOf(UDamageTypePhysical::StaticClass()))
	{
		return OriginDamage * (1 - GetStat_PhysicalResistance());
	}
	else if (DamageType->IsChildOf(UDamageTypeElemental::StaticClass()))
	{
		return OriginDamage * (1 - GetStat_ElementalResistance());
	}
	else if (DamageType->IsChildOf(UDamageTypeMagical::StaticClass()))
	{
		return OriginDamage * (1 - GetStat_MagicalResistance());
	}

	UE_LOG(LogTemp, Warning, TEXT("Server_ModifyDamageByType - Unknown damage type!"));
	return OriginDamage;
}

void AGameCharacter::ProcessMeleeAtk(AGameCharacter* Target, const FHitResult& MeleeHitResult)
{
	if (!HasAuthority()) return;
	if (!IsValid(Target)) return;
	int Damage = BaseMeleeDamage + FMath::RandRange(0.0f, AddMeleeDamage); // 정수 데미지

	// 근접공격 시작점을 실제 트레이스 시작점으로 처리해 데미지 적용을 할 경우 히트 서클이 부자연스러운 방향으로 생성됨
	FHitResult ModifiedHitRes = MeleeHitResult;
	ModifiedHitRes.TraceStart = this->GetActorLocation();

	UGameplayStatics::ApplyPointDamage(Target, Damage, ModifiedHitRes.ImpactNormal, ModifiedHitRes, GetController(), this, MeleeDamageType);
}

void AGameCharacter::Multicast_PlayItemFxRPC_Implementation(bool bIsPrimary, APlayerController* InvokeHost)
{
	if (InvokeHost && InvokeHost->IsLocalPlayerController())
	{
		// 최초 재생 주체는 이미 FX를 처리했으므로 무시한다
		return;
	}

	Local_PlayItemFx(bIsPrimary);
}

void AGameCharacter::Host_RegisterFire(bool bIsPrimary)
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		Client_SimulateFire(bIsPrimary);
	}
	Local_PlayItemFx(bIsPrimary);
	Server_RegisterFireRPC(bIsPrimary);
}

void AGameCharacter::Client_SimulateFire(bool bIsPrimary)
{
	if (HasAuthority()) return; // 클라만 가능
	if (EquipSystem)
	{
		AWieldItem* CurrWeaponActor = EquipSystem->GetCurrWeaponActor();
		if (IsValid(CurrWeaponActor))
		{
			// 격발했다고 가정하고 값을 변경한다
			// 서버로부터 격발 처리 완료 요청이 돌아오기 전까지 이 값은 false를 유지한다
			if (bIsPrimary && IsValid(CurrWeaponActor->PrimaryActComponent))
			{
				CurrWeaponActor->PrimaryActComponent->Local_OnClientSimulation(this);
			}
			else if (!bIsPrimary && IsValid(CurrWeaponActor->SecondaryActComponent))
			{
				CurrWeaponActor->SecondaryActComponent->Local_OnClientSimulation(this);
			}
		}
	}
}

bool AGameCharacter::Host_CanPerformFire(bool bIsPrimary)
{
	if (EquipSystem)
	{
		AWieldItem* CurrWeaponActor = EquipSystem->GetCurrWeaponActor();
		if (IsValid(CurrWeaponActor))
		{
			// 컴포넌트 트리거 가능 여부 확인
			if (bIsPrimary && IsValid(CurrWeaponActor->PrimaryActComponent))
			{
				return CurrWeaponActor->PrimaryActComponent->Host_CanTrigger(this);
			}
			else if (!bIsPrimary && IsValid(CurrWeaponActor->SecondaryActComponent))
			{
				return CurrWeaponActor->SecondaryActComponent->Host_CanTrigger(this);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Host_CanPerformFire - CurrWeaponActor is not valid!"));
		}
	}
	return false;
}

void AGameCharacter::Server_InteractRPC_Implementation()
{
	if (HasAuthority())
	{
		const TArray<FHitResult>& HitResult = Reach();
		if (HitResult.IsEmpty())
		{
			return;
		}

		for (int32 Idx = HitResult.Num() - 1; Idx >= 0; --Idx)
		{
			AActor* HitActor = HitResult[Idx].GetActor();
			ABaseItem* HitItem = Cast<ABaseItem>(HitActor);
			AMuzzleTriggeredActor* HitMuzzleTriggeredActor = Cast<AMuzzleTriggeredActor>(HitActor);

			// 다중 상속을 고려하지 않음
			if (HitItem)
			{
				Server_ProcessPickup(HitItem);
				break;
			}
			else if (HitMuzzleTriggeredActor)
			{
				Server_ProcessMuzzleTrigger(HitMuzzleTriggeredActor);
				break;
			}
			else if (HitActor)
			{
				// 액터는 존재하지만 인터랙션이 불가능한 경우
				UE_LOG(LogTemp, Warning, TEXT("Server_InteractRPC_Implementation - Unable to process interaction with %s"), *(HitActor->GetName()));
			}
			else
			{
				// 액터가 존재하지 않는 경우
				UE_LOG(LogTemp, Error, TEXT("Server_InteractRPC_Implementation - Unexpected error; Actor is null!"));
			}
		}
	}
	return;
}

TPair<FVector, FRotator> AGameCharacter::GetHandPointInfo()
{
	// 카메라가 없기 때문에 APawun::GetActorEyesViewPoint를 그대로 사용한다
	FVector Location;
	FRotator Rotation;
	APawn::GetActorEyesViewPoint(Location, Rotation);
	Location += Rotation.Vector() * GrabOffset;
	return TPair<FVector, FRotator>(Location, Rotation);
}

TPair<FVector, FRotator> AGameCharacter::GetMuzzleInfo()
{
	// TODO: AI의 머즐 로직 제작
	UE_LOG(LogTemp, Error, TEXT("Unable to get MuzzleInfo!"));
	return TPair<FVector, FRotator>();
}

FVector AGameCharacter::GetTopOfHeadLocation()
{
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (!CapsuleComp) return FVector::ZeroVector;
	return CapsuleComp->GetComponentLocation() + FVector(0, 0, CapsuleComp->GetScaledCapsuleHalfHeight());
}

bool AGameCharacter::IsInAir() const
{
	UCharacterMovementComponent* TRMoveComp = GetCharacterMovement();
	if (!TRMoveComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("IsInAir - MoveComp is null!"));
		return false;
	}
	return TRMoveComp->IsFalling();
}

void AGameCharacter::InitHitbox()
{
	// 소켓에 히트박스 어태치
	SetupHitboxToBone();

	// 히트박스 부위 별 데미지 초기화
	InitHitboxDmgMultipliers();
}

void AGameCharacter::SetupHitboxToBone()
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	HeadColComponent->SetupAttachment(SkeletalMesh, FName(HeadSockName));
	TorsoColComponent->SetupAttachment(SkeletalMesh, FName(TorsoSockName));
	PelvisColComponent->SetupAttachment(SkeletalMesh, FName(PelvisSockName));
	RArmUpperColComponent->SetupAttachment(SkeletalMesh, FName(RArmUpperSockName));
	RArmLowerColComponent->SetupAttachment(SkeletalMesh, FName(RArmLowerSockName));
	RHandColComponent->SetupAttachment(SkeletalMesh, FName(RHandSockName));
	LArmUpperColComponent->SetupAttachment(SkeletalMesh, FName(LArmUpperSockName));
	LArmLowerColComponent->SetupAttachment(SkeletalMesh, FName(LArmLowerSockName));
	LHandColComponent->SetupAttachment(SkeletalMesh, FName(LHandSockName));
	RLegUpperColComponent->SetupAttachment(SkeletalMesh, FName(RLegUpperSockName));
	RLegLowerColComponent->SetupAttachment(SkeletalMesh, FName(RLegLowerSockName));
	RFootColComponent->SetupAttachment(SkeletalMesh, FName(RFootSockName));
	LLegUpperColComponent->SetupAttachment(SkeletalMesh, FName(LLegUpperSockName));
	LLegLowerColComponent->SetupAttachment(SkeletalMesh, FName(LLegLowerSockName));
	LFootColComponent->SetupAttachment(SkeletalMesh, FName(LFootSockName));
}

void AGameCharacter::SnapHitboxToBone()
{
	// 트랜스폼을 곧장 어태치 위치로 이동시킨다
	// 이는 이 함수가 호출되는 틱에서 바로 히트박스 트랜스폼 정보를 사용하기 위함이다
	SetHitboxTransformToBoneImmediate();
}

void AGameCharacter::SetHitboxTransformToBoneSingle(UHitboxComponent* Hitbox, USkeletalMeshComponent* TargetMesh, FName TargetBoneName, FTransform RelativeTransform)
{
	FTransform TargetTransform = TargetMesh->GetBoneTransform(TargetMesh->GetBoneIndex(TargetBoneName));
	FQuat ParentRotation = TargetTransform.GetRotation();
	Hitbox->SetWorldLocationAndRotationNoPhysics(TargetTransform.GetLocation() + ParentRotation.RotateVector(RelativeTransform.GetLocation()), FRotator(ParentRotation * RelativeTransform.GetRotation()));
}

void AGameCharacter::SetHitboxTransformToBoneImmediate()
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	SetHitboxTransformToBoneSingle(HeadColComponent, SkeletalMesh, FName(HeadSockName), DeltaRelativeHead);
	SetHitboxTransformToBoneSingle(TorsoColComponent, SkeletalMesh, FName(TorsoSockName), DeltaRelativeTorso);
	SetHitboxTransformToBoneSingle(PelvisColComponent, SkeletalMesh, FName(PelvisSockName), DeltaRelativePelvis);
	SetHitboxTransformToBoneSingle(RArmUpperColComponent, SkeletalMesh, FName(RArmUpperSockName), DeltaRelativeRArmUpper);
	SetHitboxTransformToBoneSingle(RArmLowerColComponent, SkeletalMesh, FName(RArmLowerSockName), DeltaRelativeRArmLower);
	SetHitboxTransformToBoneSingle(RHandColComponent, SkeletalMesh, FName(RHandSockName), DeltaRelativeRHand);
	SetHitboxTransformToBoneSingle(LArmUpperColComponent, SkeletalMesh, FName(LArmUpperSockName), DeltaRelativeLArmUpper);
	SetHitboxTransformToBoneSingle(LArmLowerColComponent, SkeletalMesh, FName(LArmLowerSockName), DeltaRelativeLArmLower);
	SetHitboxTransformToBoneSingle(LHandColComponent, SkeletalMesh, FName(LHandSockName), DeltaRelativeLHand);
	SetHitboxTransformToBoneSingle(RLegUpperColComponent, SkeletalMesh, FName(RLegUpperSockName), DeltaRelativeRLegUpper);
	SetHitboxTransformToBoneSingle(RLegLowerColComponent, SkeletalMesh, FName(RLegLowerSockName), DeltaRelativeRLegLower);
	SetHitboxTransformToBoneSingle(RFootColComponent, SkeletalMesh, FName(RFootSockName), DeltaRelativeRFoot);
	SetHitboxTransformToBoneSingle(LLegUpperColComponent, SkeletalMesh, FName(LLegUpperSockName), DeltaRelativeLLegUpper);
	SetHitboxTransformToBoneSingle(LLegLowerColComponent, SkeletalMesh, FName(LLegLowerSockName), DeltaRelativeLLegLower);
	SetHitboxTransformToBoneSingle(LFootColComponent, SkeletalMesh, FName(LFootSockName), DeltaRelativeLFoot);
}

// Called every frame
void AGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DeferredTaskTick(DeltaTime);
}

// Called to bind functionality to input
void AGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AGameCharacter::Server_OnRoomEnter(ARoomLevel* RoomLevel)
{
	LastEnteredRoomLevel = RoomLevel;
	//TR_PRINT_FSTRING("Server_OnRoomEnter - %s", *(GetName()));
}

void AGameCharacter::Server_OnRoomExit(ARoomLevel* RoomLevel)
{
	// Exit 시 LastEnteredRoom을 수정하진 않는다
	//TR_PRINT_FSTRING("Server_OnRoomExit - %s", *(GetName()));
}

ARoomLevel* AGameCharacter::GetLastEnteredRoomLevel() const
{
	if (!RoomObserverComp || !RoomObserverComp->IsActive()) return nullptr;
	if (!IsValid(LastEnteredRoomLevel)) return nullptr;
	return LastEnteredRoomLevel;
}

void AGameCharacter::Multicast_Roll_Implementation(float Forward, float Right)
{
	TR_PRINT("Multicast_Roll()");
	// TODO
}

UAnimMontage* AGameCharacter::GetDeployAnimMontage(EHumanoidWeaponState WeaponState)
{
	// 들어온 아이템의 종류에 따라 적절한 애니메이션 맵핑
	if (WeaponState == EHumanoidWeaponState::LIGHT) return AnimConfig->AM_DeployPistol;
	else if (WeaponState == EHumanoidWeaponState::HEAVY) return AnimConfig->AM_DeployRifle;
	return AnimConfig->AM_GenericUnequip;
}

void AGameCharacter::PlayDeployAnimMontage()
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (SkeletalMesh)
	{
		if (UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance())
		{
			UAnimMontage* Montage = GetDeployAnimMontage(this->WeaponEquipState);
			if (!Montage)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cannot find Item Deploy AnimMontage."));
				return;
			}

			// 재생 후 바인딩
			AnimInst->Montage_Play(Montage);

			// 매번 새롭게 바인딩
			FOnMontageEnded DeployMontageEnded;
			DeployMontageEnded.BindUObject(this, &AGameCharacter::EndDeployAnimMontage);
			AnimInst->Montage_SetEndDelegate(DeployMontageEnded, Montage);
		}
	}
}

void AGameCharacter::EndDeployAnimMontage(UAnimMontage* Montage, bool bInterrupted)
{
	// 서버일 경우 추가 로직 실행
	if (HasAuthority())
	{
		return;
	}
}

void AGameCharacter::Server_SwitchAnimClassLayerRPC_Implementation(EAnimClassType ClassType)
{
	if (!HasAuthority()) return;
	Multicast_SwitchAnimClassLayerRPC(ClassType);
}

void AGameCharacter::Multicast_SwitchAnimClassLayerRPC_Implementation(EAnimClassType ClassType)
{
	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh)
	{
		TSubclassOf<UAnimInstance> AnimClass = GetAnimClassOfType(ClassType);
		if (AnimClass)
		{
			SkelMesh->LinkAnimClassLayers(AnimClass);
			//UE_LOG(LogTemp, Error, TEXT("AnimClass Set to %s"), *(AnimClass->GetName()));
			return;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Multicast_SwitchAnimClassLayerRPC - Character %s failed to switch animclass!"), *(GetName()));
}

TSubclassOf<UAnimInstance> AGameCharacter::GetAnimClassOfType(EAnimClassType ClassType)
{
	// 필요 시 오버라이드
	if (!AnimConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("GetAnimClassOfType - Character %s has no anim config."), *(GetName()));
		return nullptr;
	}

	switch (ClassType)
	{
	case EAnimClassType::AC_UNARMED:
		return AnimConfig->UnarmedAnimClass;
	case EAnimClassType::AC_LIGHTWEAPON:
		return AnimConfig->LightWeaponAnimClass;
	case EAnimClassType::AC_HEAVYWEAPON:
		return AnimConfig->HeavyWeaponAnimClass;
	}
	return nullptr;
}

void AGameCharacter::Server_Ragdollfy()
{
	Multicast_RagdollfyRPC();
}

void AGameCharacter::Multicast_RagdollfyRPC_Implementation()
{
	DisableAllComponentsExceptSkeletalMesh();

	// 로컬 피직스 가동
	USkeletalMeshComponent* SkelMesh = this->GetMesh();
	if (IsValid(SkelMesh) && IsValid(SkelMesh->GetPhysicsAsset()))
	{
		SkelMesh->SetSimulatePhysics(true);
		SkelMesh->SetEnableGravity(true);
		SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		SkelMesh->SetGenerateOverlapEvents(false);
		SkelMesh->SetShouldUpdatePhysicsVolume(true); // 래그돌은 피직스에 영향을 준다

		TRUtils::OptimizePrimitiveComp(SkelMesh, false, false/*라이팅은 허용*/);
	}
}

void AGameCharacter::DisableAllComponentsExceptSkeletalMesh()
{
	// 스켈레탈 메쉬를 제외한 모든 컴포넌트 작동 중지
	USkeletalMeshComponent* SkelMesh = GetMesh();
	TSet<UActorComponent*> CharComps = GetComponents();
	for (UActorComponent* Component : CharComps)
	{
		if (Component != SkelMesh)
		{
			Component->SetIsReplicated(false); // 래그돌은 로컬
			Component->Deactivate();
			Component->SetComponentTickEnabled(false);
			if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
			{
				PrimitiveComponent->SetVisibility(false);
				PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				PrimitiveComponent->SetSimulatePhysics(false);
			}
		}
	}
	GetCharacterMovement()->DisableMovement();
}

void AGameCharacter::InitHitboxDmgMultipliers()
{
	SetHitboxDmgMultiplier(HeadColComponent, HeadDmgMult);
	SetHitboxDmgMultiplier(TorsoColComponent, TorsoDmgMult);
	SetHitboxDmgMultiplier(PelvisColComponent, PelvisDmgMult);

	SetHitboxDmgMultiplier(RArmUpperColComponent, RArmUpperDmgMult);
	SetHitboxDmgMultiplier(RArmLowerColComponent, RArmLowerDmgMult);
	SetHitboxDmgMultiplier(RHandColComponent, RHandDmgMult);

	SetHitboxDmgMultiplier(LArmUpperColComponent, LArmUpperDmgMult);
	SetHitboxDmgMultiplier(LArmLowerColComponent, LArmLowerDmgMult);
	SetHitboxDmgMultiplier(LHandColComponent, LHandDmgMult);

	SetHitboxDmgMultiplier(RLegUpperColComponent, RLegUpperDmgMult);
	SetHitboxDmgMultiplier(RLegLowerColComponent, RLegLowerDmgMult);
	SetHitboxDmgMultiplier(RFootColComponent, RFootDmgMult);

	SetHitboxDmgMultiplier(LLegUpperColComponent, LLegUpperDmgMult);
	SetHitboxDmgMultiplier(LLegLowerColComponent, LLegLowerDmgMult);
	SetHitboxDmgMultiplier(LFootColComponent, LFootDmgMult);
}

void AGameCharacter::SetHitboxDmgMultiplier(UHitboxComponent* Hitbox, float Multiplier)
{
	if (!IsValid(Hitbox)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("SetHitboxDmgMultiplier - Hitbox is invalid! Check if the function is being called after creating the subobject."));
		return;
	}
	Hitbox->SetDamageMultiplier(Multiplier);
}

void AGameCharacter::ActivateDetailedHitboxFor(float Duration)
{
	if (bIsDetailedHitboxActivated)
	{
		// 이미 세부 본들이 활성화 되어있다면 타이머 리셋 후 갱신
		GetWorld()->GetTimerManager().ClearTimer(DetailedHitboxDeactivateTimer);
		GetWorld()->GetTimerManager().SetTimer(DetailedHitboxDeactivateTimer, this, &AGameCharacter::DeactivateDetailedHitbox, Duration, false);
		return;
	}

	bIsDetailedHitboxActivated = true;
	ActivateDetailedHitbox();

	// 디액티베이션 콜백 설정
	GetWorld()->GetTimerManager().SetTimer(DetailedHitboxDeactivateTimer, this, &AGameCharacter::DeactivateDetailedHitbox, Duration, false);
}

void AGameCharacter::ActivateDetailHitboxForTick()
{
	if (bIsDetailedHitboxActivated) return; // 한 틱 이상 유지되지 않으므로 타이머 갱신 및 리셋 필요 없음

	bIsDetailedHitboxActivated = true;
	ActivateDetailedHitbox();
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AGameCharacter::DeactivateDetailedHitbox);
}

void AGameCharacter::ActivateDetailedHitbox()
{
	// 본에 부착시의 추정 위치로 이동한다
	SnapHitboxToBone();

	// 전체 활성화
	for (UHitboxComponent* DetailedHitbox : DetailColComponents)
	{
		DetailedHitbox->Activate(true/*리셋 필요*/);
		DetailedHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	//////////////TEMP
	/*for (UHitboxComponent* DetailedHitbox : DetailColComponents)
	{
		DrawDebugBox(GetWorld(), DetailedHitbox->GetComponentLocation(), DetailedHitbox->GetScaledBoxExtent(), DetailedHitbox->GetComponentQuat(), FColor::Cyan, false, 5.0f);
	}*/
}

void AGameCharacter::CacheHitboxDeltaRelativeTransform()
{
	DeltaRelativeHead = HeadColComponent->GetRelativeTransform();
	DeltaRelativeTorso = TorsoColComponent->GetRelativeTransform();
	DeltaRelativePelvis = PelvisColComponent->GetRelativeTransform();

	DeltaRelativeRArmUpper = RArmUpperColComponent->GetRelativeTransform();
	DeltaRelativeRArmLower = RArmLowerColComponent->GetRelativeTransform();
	DeltaRelativeRHand = RHandColComponent->GetRelativeTransform();

	DeltaRelativeLArmUpper = LArmUpperColComponent->GetRelativeTransform();
	DeltaRelativeLArmLower = LArmLowerColComponent->GetRelativeTransform();
	DeltaRelativeLHand = LHandColComponent->GetRelativeTransform();

	DeltaRelativeRLegUpper = RLegUpperColComponent->GetRelativeTransform();
	DeltaRelativeRLegLower = RLegLowerColComponent->GetRelativeTransform();
	DeltaRelativeRFoot = RFootColComponent->GetRelativeTransform();

	DeltaRelativeLLegUpper = LLegUpperColComponent->GetRelativeTransform();
	DeltaRelativeLLegLower = LLegLowerColComponent->GetRelativeTransform();
	DeltaRelativeLFoot = LFootColComponent->GetRelativeTransform();
}

void AGameCharacter::DeactivateDetailedHitbox()
{
	// 활성화에서 비활성화로 전환되었을 경우
	if (bIsDetailedHitboxActivated)
	{
		bIsDetailedHitboxActivated = false;

		// 타이머 리셋
		GetWorld()->GetTimerManager().ClearTimer(DetailedHitboxDeactivateTimer);
	}

	// 전체 비활성화
	for (UHitboxComponent* DetailedHitbox : DetailColComponents)
	{
		DetailedHitbox->Deactivate();
		DetailedHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	//////////////TEMP
	/*for (UHitboxComponent* DetailedHitbox : DetailColComponents)
	{
		DrawDebugBox(GetWorld(), DetailedHitbox->GetComponentLocation(), DetailedHitbox->GetScaledBoxExtent(), DetailedHitbox->GetComponentQuat(), FColor::Red, false, 5.0f);
	}*/
}

void AGameCharacter::Server_DropAllItemFromInv(float RandomOffset, bool bDeferred)
{
	if (!HasAuthority()) return;
	if (IsValid(InvComponent))
	{
		if (bDeferred)
		{
			InvComponent->Server_DropAllInvObjectDeferred(GetActorLocation(), RandomOffset);
		}
		else
		{
			InvComponent->Server_DropAllInvObjectImmediate(GetActorLocation(), RandomOffset);
		}
	}
}

FName AGameCharacter::GetSockNameOfPart(ECharacterParts Part)
{
	// 기본값은 인간형 캐릭터의 경우를 가정해 제작한다
	// 필요 시 오버라이드 해 구현한다
	switch (Part)
	{
	case ECharacterParts::ECP_CentralNerves:
		return TryGetSocketName("head");
	case ECharacterParts::ECP_PrimaryWield:
		return TryGetSocketName("hand_r");
	case ECharacterParts::ECP_SecondaryWield:
		return TryGetSocketName("hand_l");
	}
	return TryGetSocketName("INVALID");
}

FName AGameCharacter::TryGetSocketName(FName SocketName)
{
	if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
	{
		if (SkeletalMesh->DoesSocketExist(SocketName)) return SocketName;
	}
	return FName("root"); // 루트 소켓 반환
}

void AGameCharacter::Server_DropAllEquippedItems(float RandomOffset, bool bDeferred)
{
	if (!HasAuthority()) return;
	if (IsValid(EquipSystem))
	{
		EquipSystem->Server_RetrieveCurrWeapon(); // NOTE: Deploy 된 무기가 없을 수 있음

		if (bDeferred)
		{
			EquipSystem->Server_DropAllInvObjectDeferred(GetActorLocation(), RandomOffset);
		}
		else
		{
			EquipSystem->Server_DropAllInvObjectImmediate(GetActorLocation(), RandomOffset);
		}
	}
}

void AGameCharacter::Local_PlayItemFx(bool bIsPrimary)
{
	if (EquipSystem)
	{
		AWieldItem* CurrWeaponActor = EquipSystem->GetCurrWeaponActor();
		if (IsValid(CurrWeaponActor))
		{
			// FX 플레이
			if (bIsPrimary && IsValid(CurrWeaponActor->PrimaryActComponent))
			{
				CurrWeaponActor->PrimaryActComponent->Local_PlayFx(this);
			}
			else if (!bIsPrimary && IsValid(CurrWeaponActor->SecondaryActComponent))
			{
				CurrWeaponActor->SecondaryActComponent->Local_PlayFx(this);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Local_PlayItemFx - CurrWeaponActor is not valid!"));
		}
	}
}

void AGameCharacter::Local_StopItemFx(bool bIsPrimary)
{
	if (EquipSystem)
	{
		AWieldItem* CurrWeaponActor = EquipSystem->GetCurrWeaponActor();
		if (IsValid(CurrWeaponActor))
		{
			// FX 중지
			if (bIsPrimary && IsValid(CurrWeaponActor->PrimaryActComponent))
			{
				CurrWeaponActor->PrimaryActComponent->Local_StopFx(this);
			}
			else if (!bIsPrimary && IsValid(CurrWeaponActor->SecondaryActComponent))
			{
				CurrWeaponActor->SecondaryActComponent->Local_StopFx(this);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Local_StopItemFx - CurrWeaponActor is not valid!"));
		}
	}
}

void AGameCharacter::Local_InitRecoil(const FRecoilAnimData Data, const float Rate, const int Bursts)
{
	if (RecoilComponent)
	{
		RecoilComponent->Init(Data, Rate, Bursts);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Local_InitRecoil - RecoilComp is not valid!"));
	}
}

void AGameCharacter::BindHUD(UTRHUDWidget* HUD)
{
	if (!HUD) return;
	if (HUD->HudTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("AGameCharacter::BindHUD - Tried to bind HUD that is already bound to other character %s."), *HUD->HudTarget->GetName());
		return;
	}
	this->Local_BoundHUDWidget = HUD;
}

void AGameCharacter::UnbindHUD()
{
	this->Local_BoundHUDWidget = nullptr;
}

void AGameCharacter::Multicast_StopItemFxRPC_Implementation(bool bIsPrimary, APlayerController* InvokeHost)
{
	if (InvokeHost && InvokeHost->IsLocalPlayerController())
	{
		// 최초 재생 주체는 이미 FX를 처리했으므로 무시한다
		return;
	}

	Local_StopItemFx(bIsPrimary);
}
