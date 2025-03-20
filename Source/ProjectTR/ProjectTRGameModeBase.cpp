// Copyright (C) 2024 by Haguk Kim


#include "ProjectTRGameModeBase.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "BotCharacter.h"
#include "TRPlayerController.h"
#include "TRGameState.h"
#include "TRPlayerState.h"
#include "TRSpectatorPawn.h"
#include "TRGameInstance.h"
#include "AIControllerPool.h"
#include "BaseItem.h"
#include "GunItem.h"
#include "TRToken.h"
#include "IconStageActor.h"
#include "DungeonActor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "TRGunPartHeaders.h"
#include "TRDungeonGenerator.h"
#include "BossDungeonGenerator.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "EngineUtils.h"
#include "RoomLevel.h"
#include "Room.h"


//TEMP
#include "GPC_TestReceiver.h"
#include "GPC_TestBarrel.h"

AProjectTRGameModeBase::AProjectTRGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AIPool = CreateDefaultSubobject<UAIControllerPool>(TEXT("AIPool")); // StartPlay에서 초기화

	// 세부 프로퍼티는 블루프린트에서 직접 지정
}

void AProjectTRGameModeBase::StartPlay()
{
	Super::StartPlay();

	check(GEngine != nullptr);
	check(DefaultGunItemClass != nullptr);

	// 파츠 정보 초기화
	InitPartsList();

	// 던전 제너레이터 바인딩
	FindAndBindDungeonGenerator();
	if (DungeonGenerator && DungeonGenerator->IsA<ABossDungeonGenerator>())
	{
		bIsLevelBossFight = true;
	}

	// AI컨트롤러 풀 초기화
	AIPool->Initialize(MaxEnemyCount);
}

void AProjectTRGameModeBase::Tick(float DeltaTime)
{
	// 던전 로직은 던전 생성이 완료된 이후에만 실행한다
	if (!DungeonGenerator) return;
	if (!DungeonGenerator->IsDungeonGenerationComplete())
	{
		DungeonGenerator->UpdateDungeonGenerationState();
		return;
	}
	if (!bDungeonGenerationDataReceived)
	{
		return;
	}

	if (bIsLevelBossFight)
	{
		BossfightTick(DeltaTime);
	}
	else
	{
		DungeonTick(DeltaTime);
	}
}

void AProjectTRGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UTRGameInstance* TRInst = GetTRGameInstance();

	// 레벨 트랜지션 발생 시 캐릭터 정보 캐싱 및 복구
	ATRPlayerController* TRPlayer = Cast<ATRPlayerController>(NewPlayer);
	if (IsValid(TRPlayer))
	{
		AGameCharacter* TRPawn = Cast<AGameCharacter>(TRPlayer->GetPawn());
		if (TRPawn && TRInst)
		{
			// 서버에 캐싱된 인스턴스 데이터를 기반으로 플레이어 인스턴스를 복구한다
			// 만약 캐싱된 데이터가 없을 경우(최초 접속 등) 아무 것도 처리하지 않는다
			FGameCharacterInstanceData* CachedData = TRInst->Server_GetCachedPlayerInstanceData(TRPlayer);
			if (CachedData)
			{
				if (!TRPawn->Server_RestoreFromInstanceData(*CachedData))
				{
					UE_LOG(LogTemp, Error, TEXT("PostLogin - Instance restoration failed for %s!"), *(TRPawn->GetName()));
				}
			}
		}
	}

	// 층수 설정
	if (TRInst)
	{
		InitDungeonDepth(TRInst->Server_GetDungeonDepth());
	}
}

UTRGameInstance* AProjectTRGameModeBase::GetTRGameInstance()
{
	return Cast<UTRGameInstance>(GetGameInstance());
}

void AProjectTRGameModeBase::UpdatePlayerNames()
{
	const TArray<ATRPlayerController*>& PlayerControllers = GetPlayersConnected();

	// 이름 업데이트
	for (ATRPlayerController* PlayerController : PlayerControllers)
	{
		AFPSCharacter* PlayerPawn = Cast<AFPSCharacter>(PlayerController->GetPawn());
		if (PlayerPawn && PlayerController->PlayerState)
		{
			PlayerPawn->Multicast_SetNameTagText(PlayerController->PlayerState->GetPlayerName());
		}
	}
}

ABaseItem* AProjectTRGameModeBase::SpawnItem(TSubclassOf<ABaseItem> ItemClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params)
{
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	ABaseItem* SpawnedItem = Cast<ABaseItem>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, ItemClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	// 생성자 이후 추가 로직

	SpawnedItem = Cast<ABaseItem>(UGameplayStatics::FinishSpawningActor(SpawnedItem, SpawnTransform));
	return SpawnedItem;
}

ABaseItem* AProjectTRGameModeBase::SpawnDecorativeItem(UInvObject* InvObj, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params)
{
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	ABaseItem* SpawnedItem = Cast<ABaseItem>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, InvObj->GetBaseItemClass(), SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	// 생성자 이후 추가 로직
	SpawnedItem->RestoreFromItemData(InvObj->GetItemData());
	if (SpawnedItem)
	{
		// 아이콘 생성 해제
		// NOTE: 이걸 하지 않을 경우 BeginPlay에서 아이콘 생성 -> DecorativeItem 생성 -> BeginPlay -> ...
		// 무한 재귀호출에 걸린다
		// 클라이언트 레플리케이션을 위해 PostInit 단계에서 처리한다
		SpawnedItem->bShouldInitializeIcon = false;

		// TODO: 기타 무효화 로직 필요 시 추가
	}

	SpawnedItem = Cast<ABaseItem>(UGameplayStatics::FinishSpawningActor(SpawnedItem, SpawnTransform));

	// 피직스, 콜리전 해제
	// NOTE: 피직스 설정을 덮어쓰기 위해서는 PostInitialization 이후 호출해야 한다
	UPrimitiveComponent* PhysComp = SpawnedItem->GetPhysComponent();
	if (PhysComp)
	{
		PhysComp->RegisterComponent();
		SpawnedItem->SetItemPhysicsTo(false);
		SpawnedItem->SetItemGravityTo(false);
		PhysComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PhysComp->ReregisterComponent();
	}

	SpawnedItem->SetActorTickEnabled(false);
	return SpawnedItem;
}

ABaseItem* AProjectTRGameModeBase::RespawnItem(TSubclassOf<class ABaseItem> ItemClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params, UInvObject* InvObject, UItemData* ItemData)
{
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	ABaseItem* SpawnedItem = Cast<ABaseItem>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, ItemClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	// 생성자 이후 추가 로직
	SpawnedItem->RestoreFromItemData(ItemData);

	SpawnedItem = Cast<ABaseItem>(UGameplayStatics::FinishSpawningActor(SpawnedItem, SpawnTransform));
	return SpawnedItem;
}

AGunItem* AProjectTRGameModeBase::SpawnRandomizedGunItem(UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params, int32 TokenTier)
{
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	AGunItem* GunItem = Cast<AGunItem>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, DefaultGunItemClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
	if (!GunItem) return nullptr;

	// 총기 랜덤 생성
	UGunPartComponent* ReceiverComp = nullptr;
	UGunPartComponent* BarrelComp = nullptr;
	UGunPartComponent* GripComp = nullptr;
	UGunPartComponent* MagazineComp = nullptr;
	UGunPartComponent* MuzzleComp = nullptr;
	UGunPartComponent* SightComp = nullptr;
	UGunPartComponent* StockComp = nullptr;

	// 파츠 선택 및 오브젝트 생성
	//const FGunPartTierData& TierData = RandomizePartsTier(TokenTier);
	//RandomizeGunParts(TierData, GunItem, &ReceiverComp, &BarrelComp, &GripComp, &MagazineComp, &MuzzleComp, &SightComp, &StockComp);
	///////////////TESTING
	ReceiverComp = NewObject<UGPC_TestReceiver>(GunItem);
	BarrelComp = NewObject<UGPC_TestBarrel>(GunItem);

	// 파츠 부착
	GunItem->SetReceiver(ReceiverComp);
	GunItem->SetBarrel(BarrelComp);
	GunItem->SetGrip(GripComp);
	GunItem->SetMagazine(MagazineComp);
	GunItem->SetMuzzle(MuzzleComp);
	GunItem->SetSight(SightComp);
	GunItem->SetStock(StockComp);

	GunItem = Cast<AGunItem>(UGameplayStatics::FinishSpawningActor(GunItem, SpawnTransform));
	return GunItem;
}

int32 AProjectTRGameModeBase::RandomizeTokenTier(int32 PlayerLevel)
{
	/////////TESTING
	// TODO: 벨런스 조정
	return FMath::Max(PlayerLevel, 3);
}

FGunPartTierData AProjectTRGameModeBase::RandomizePartsTier(int32 TokenTier)
{
	// TODO: 벨런스 조정
	return FGunPartTierData();
}

void AProjectTRGameModeBase::RandomizeGunParts(const FGunPartTierData& TierData, AGunItem* GunItem, UGunPartComponent** ReceiverComp, UGunPartComponent** BarrelComp, UGunPartComponent** GripComp, UGunPartComponent** MagazineComp, UGunPartComponent** MuzzleComp, UGunPartComponent** SightComp, UGunPartComponent** StockComp)
{
	// 필수 파츠들의 경우 반드시 생성되어야 하므로 다운그레이드가 가능라도록 파라미터를 true로 설정한다
	*ReceiverComp = SelectAndCreatePart(GunItem, EGunPartType::EGT_Receiver, TierData.ReceiverTier, true);
	*BarrelComp = SelectAndCreatePart(GunItem, EGunPartType::EGT_Barrel, TierData.BarrelTier, true);
	*GripComp = SelectAndCreatePart(GunItem, EGunPartType::EGT_Grip, TierData.GripTier, true);
	*MagazineComp = SelectAndCreatePart(GunItem, EGunPartType::EGT_Magazine, TierData.MagazineTier, true);

	TArray<UGunPartComponent*> PartsUsed;
	if (ReceiverComp && *ReceiverComp) PartsUsed.Add(*ReceiverComp);
	if (BarrelComp && *BarrelComp) PartsUsed.Add(*BarrelComp);
	if (GripComp && *GripComp) PartsUsed.Add(*GripComp);
	if (MagazineComp && *MagazineComp) PartsUsed.Add(*MagazineComp);

	float MuzzleGenChance = 0.0f;
	float SightGenChance = 0.0f;
	float StockGenChance = 0.0f;
	for (const UGunPartComponent* Part : PartsUsed)
	{
		MuzzleGenChance += Part->GetMuzzleGenChanceDelta();
		SightGenChance += Part->GetSightGenChanceDelta();
		StockGenChance += Part->GetStockGenChanceDelta();
	}

	if (FMath::FRand() <= MuzzleGenChance)
	{
		*MuzzleComp = SelectAndCreatePart(GunItem, EGunPartType::EGT_Muzzle, TierData.MuzzleTier, false);
	}
	if (FMath::FRand() <= SightGenChance)
	{
		*SightComp = SelectAndCreatePart(GunItem, EGunPartType::EGT_Sight, TierData.SightTier, false);
	}
	if (FMath::FRand() <= StockGenChance)
	{
		*StockComp = SelectAndCreatePart(GunItem, EGunPartType::EGT_Stock, TierData.StockTier, false);
	}
}

UGunPartComponent* AProjectTRGameModeBase::SelectAndCreatePart(AGunItem* GunItem, const EGunPartType PartType, const int32 PartTier, bool bDowngradeTierIfEmpty)
{
	TPair<TArray<UClass*>*, TArray<float>*> ListPair = SelectPartsList(PartType, PartTier);
	UClass* SelectedGunPartClass = TRUtils::GetRandomElementByWeight<UClass>(*ListPair.Get<0>(), *ListPair.Get<1>());
	if (!SelectedGunPartClass)
	{
		if (bDowngradeTierIfEmpty && PartTier > 1)
		{
			return SelectAndCreatePart(GunItem, PartType, PartTier - 1, bDowngradeTierIfEmpty);
		}
		UE_LOG(LogTemp, Error, TEXT("SelectAndCreatePart - Invalid gun part class! Args: %d %d"), PartType, PartTier);
		return nullptr;
	}
	return NewObject<UGunPartComponent>(GunItem, SelectedGunPartClass);
}

void AProjectTRGameModeBase::InitPartsList()
{
	// 리시버
	AddPartToList<UGPC_RC_DoubleBarrel1>();
	AddPartToList<UGPC_RC_Handgun1>();
	AddPartToList<UGPC_RC_Pistol1>();
	AddPartToList<UGPC_RC_Pistol2>();
	AddPartToList<UGPC_RC_Revolver1>();
	AddPartToList<UGPC_RC_Rifle1>();
	AddPartToList<UGPC_RC_Rifle2>();
	AddPartToList<UGPC_RC_Rifle3>();
	AddPartToList<UGPC_RC_Rifle4>();
	AddPartToList<UGPC_RC_Shotgun1>();
	AddPartToList<UGPC_RC_Shotgun2>();
	AddPartToList<UGPC_RC_Shotgun3>();
	AddPartToList<UGPC_RC_SMG1>();
	AddPartToList<UGPC_RC_SMG2>();
	AddPartToList<UGPC_RC_SMG3>();
	AddPartToList<UGPC_RC_SMG4>();
	AddPartToList<UGPC_RC_Sniper1>();
	AddPartToList<UGPC_RC_Sniper2>();
	AddPartToList<UGPC_RC_Sniper3>();
	AddPartToList<UGPC_RC_Sniper4>();

	// 배럴
	AddPartToList<UGPC_BR_DoubleBarrel1>();
	AddPartToList<UGPC_BR_DoubleBarrel2>();
	AddPartToList<UGPC_BR_DoubleBarrel3>();
	AddPartToList<UGPC_BR_Pistol1>();
	AddPartToList<UGPC_BR_Pistol2>();
	AddPartToList<UGPC_BR_Pistol3>();
	AddPartToList<UGPC_BR_Revolver1>();
	AddPartToList<UGPC_BR_Revolver2>();
	AddPartToList<UGPC_BR_Revolver3>();
	AddPartToList<UGPC_BR_Rifle1>();
	AddPartToList<UGPC_BR_Rifle2>();
	AddPartToList<UGPC_BR_Rifle3>();
	AddPartToList<UGPC_BR_Rifle4>();
	AddPartToList<UGPC_BR_Rifle5>();
	AddPartToList<UGPC_BR_Rifle6>();
	AddPartToList<UGPC_BR_Rifle7>();
	AddPartToList<UGPC_BR_Rifle8>();
	AddPartToList<UGPC_BR_Rifle9>();
	AddPartToList<UGPC_BR_Rifle10>();
	AddPartToList<UGPC_BR_Rifle11>();
	AddPartToList<UGPC_BR_Rifle12>();
	AddPartToList<UGPC_BR_Shotgun1>();
	AddPartToList<UGPC_BR_Shotgun2>();
	AddPartToList<UGPC_BR_Shotgun3>();
	AddPartToList<UGPC_BR_Shotgun4>();
	AddPartToList<UGPC_BR_Shotgun5>();
	AddPartToList<UGPC_BR_Shotgun6>();
	AddPartToList<UGPC_BR_Shotgun7>();
	AddPartToList<UGPC_BR_Shotgun8>();
	AddPartToList<UGPC_BR_SMG1>();
	AddPartToList<UGPC_BR_SMG2>();
	AddPartToList<UGPC_BR_SMG3>();
	AddPartToList<UGPC_BR_SMG4>();
	AddPartToList<UGPC_BR_SMG5>();
	AddPartToList<UGPC_BR_SMG6>();
	AddPartToList<UGPC_BR_SMG7>();
	AddPartToList<UGPC_BR_SMG8>();
	AddPartToList<UGPC_BR_SMG9>();
	AddPartToList<UGPC_BR_SMG10>();
	AddPartToList<UGPC_BR_SMG11>();
	AddPartToList<UGPC_BR_SMG12>();
	AddPartToList<UGPC_BR_Sniper1>();
	AddPartToList<UGPC_BR_Sniper2>();
	AddPartToList<UGPC_BR_Sniper3>();
	AddPartToList<UGPC_BR_Sniper4>();
	AddPartToList<UGPC_BR_Sniper5>();
	AddPartToList<UGPC_BR_Sniper6>();
	AddPartToList<UGPC_BR_Sniper7>();
	AddPartToList<UGPC_BR_Sniper8>();
	AddPartToList<UGPC_BR_Sniper9>();

	// 그립
	AddPartToList<UGPC_GR_Pistol1>();
	AddPartToList<UGPC_GR_Pistol2>();
	AddPartToList<UGPC_GR_Revolver1>();
	AddPartToList<UGPC_GR_Revolver2>();
	AddPartToList<UGPC_GR_Revolver3>();
	AddPartToList<UGPC_GR_Rifle1>();
	AddPartToList<UGPC_GR_Rifle2>();
	AddPartToList<UGPC_GR_Rifle3>();
	AddPartToList<UGPC_GR_Rifle4>();
	AddPartToList<UGPC_GR_Rifle5>();
	AddPartToList<UGPC_GR_Rifle6>();
	AddPartToList<UGPC_GR_Rifle7>();
	AddPartToList<UGPC_GR_Rifle8>();
	AddPartToList<UGPC_GR_Rifle9>();
	AddPartToList<UGPC_GR_Shotgun1>();
	AddPartToList<UGPC_GR_Shotgun2>();
	AddPartToList<UGPC_GR_Shotgun3>();
	AddPartToList<UGPC_GR_Shotgun4>();
	AddPartToList<UGPC_GR_Shotgun5>();
	AddPartToList<UGPC_GR_SMG1>();
	AddPartToList<UGPC_GR_SMG2>();
	AddPartToList<UGPC_GR_SMG3>();
	AddPartToList<UGPC_GR_SMG4>();
	AddPartToList<UGPC_GR_SMG5>();
	AddPartToList<UGPC_GR_SMG6>();
	AddPartToList<UGPC_GR_SMG7>();
	AddPartToList<UGPC_GR_SMG8>();
	AddPartToList<UGPC_GR_Sniper1>();
	AddPartToList<UGPC_GR_Sniper2>();
	AddPartToList<UGPC_GR_Sniper3>();
	AddPartToList<UGPC_GR_Sniper4>();

	// 스톡
	AddPartToList<UGPC_ST_DoubleBarrel1>();
	AddPartToList<UGPC_ST_DoubleBarrel2>();
	AddPartToList<UGPC_ST_Pistol1>();
	AddPartToList<UGPC_ST_Rifle1>();
	AddPartToList<UGPC_ST_Rifle2>();
	AddPartToList<UGPC_ST_Rifle3>();
	AddPartToList<UGPC_ST_Rifle4>();
	AddPartToList<UGPC_ST_Rifle5>();
	AddPartToList<UGPC_ST_Rifle6>();
	AddPartToList<UGPC_ST_Rifle7>();
	AddPartToList<UGPC_ST_Rifle8>();
	AddPartToList<UGPC_ST_Rifle9>();
	AddPartToList<UGPC_ST_Rifle10>();
	AddPartToList<UGPC_ST_Shotgun1>();
	AddPartToList<UGPC_ST_Shotgun2>();
	AddPartToList<UGPC_ST_Shotgun3>();
	AddPartToList<UGPC_ST_Shotgun4>();
	AddPartToList<UGPC_ST_SMG1>();
	AddPartToList<UGPC_ST_SMG2>();
	AddPartToList<UGPC_ST_SMG3>();
	AddPartToList<UGPC_ST_SMG4>();
	AddPartToList<UGPC_ST_Sniper1>();
	AddPartToList<UGPC_ST_Sniper2>();
	AddPartToList<UGPC_ST_Sniper3>();
	AddPartToList<UGPC_ST_Sniper4>();
	AddPartToList<UGPC_ST_Sniper5>();
	AddPartToList<UGPC_ST_Sniper6>();

	// 매거진
	AddPartToList<UGPC_MG_Curved1>();
	AddPartToList<UGPC_MG_Curved2>();
	AddPartToList<UGPC_MG_CurvedStack1>();
	AddPartToList<UGPC_MG_CurvedStack2>();
	AddPartToList<UGPC_MG_Pistol1>();
	AddPartToList<UGPC_MG_Pistol2>();
	AddPartToList<UGPC_MG_Pistol3>();
	AddPartToList<UGPC_MG_Pistol4>();
	AddPartToList<UGPC_MG_Pistol5>();
	AddPartToList<UGPC_MG_Pistol6>();
	AddPartToList<UGPC_MG_Rifle1>();
	AddPartToList<UGPC_MG_Rifle2>();
	AddPartToList<UGPC_MG_Rifle3>();
	AddPartToList<UGPC_MG_Rifle4>();
	AddPartToList<UGPC_MG_Rifle5>();
	AddPartToList<UGPC_MG_Shotgun1>();
	AddPartToList<UGPC_MG_Shotgun2>();
	AddPartToList<UGPC_MG_Shotgun3>();
	AddPartToList<UGPC_MG_SMG1>();
	AddPartToList<UGPC_MG_SMG2>();
	AddPartToList<UGPC_MG_SMG3>();
	AddPartToList<UGPC_MG_SMG4>();
	AddPartToList<UGPC_MG_Sniper1>();
	AddPartToList<UGPC_MG_Sniper2>();
	AddPartToList<UGPC_MG_Sniper3>();
	AddPartToList<UGPC_MG_Sniper4>();
	AddPartToList<UGPC_MG_Sniper5>();
	AddPartToList<UGPC_MG_Sniper6>();
	
	// 머즐
	AddPartToList<UGPC_MZ_Any1>();
	AddPartToList<UGPC_MZ_Any2>();
	AddPartToList<UGPC_MZ_Any3>();
	AddPartToList<UGPC_MZ_Any4>();
	AddPartToList<UGPC_MZ_Any5>();
	AddPartToList<UGPC_MZ_Any6>();
	AddPartToList<UGPC_MZ_Any7>();
	AddPartToList<UGPC_MZ_Any8>();
	AddPartToList<UGPC_MZ_Any9>();
	AddPartToList<UGPC_MZ_Any10>();
	AddPartToList<UGPC_MZ_Any11>();
	AddPartToList<UGPC_MZ_Pistol1>();
	AddPartToList<UGPC_MZ_Pistol2>();
	AddPartToList<UGPC_MZ_Rifle1>();
	AddPartToList<UGPC_MZ_Rifle2>();
	AddPartToList<UGPC_MZ_Rifle3>();
	AddPartToList<UGPC_MZ_Shotgun1>();
	AddPartToList<UGPC_MZ_SMG1>();
	AddPartToList<UGPC_MZ_SMG2>();
	AddPartToList<UGPC_MZ_Sniper1>();
	AddPartToList<UGPC_MZ_Sniper2>();

	// 사이트
	AddPartToList<UGPC_SI_ACOG1>();
	AddPartToList<UGPC_SI_DoubleBarrel1>();
	AddPartToList<UGPC_SI_RDS1>();
	AddPartToList<UGPC_SI_RDS2>();
	AddPartToList<UGPC_SI_RDS3>();
	AddPartToList<UGPC_SI_Rifle1>();
	AddPartToList<UGPC_SI_Rifle2>();
	AddPartToList<UGPC_SI_Rifle3>();
	AddPartToList<UGPC_SI_Rifle4>();
	AddPartToList<UGPC_SI_Rifle5>();
	AddPartToList<UGPC_SI_Rifle6>();
	AddPartToList<UGPC_SI_Rifle7>();
	AddPartToList<UGPC_SI_Scope1>();
	AddPartToList<UGPC_SI_Scope2>();
	AddPartToList<UGPC_SI_Shotgun1>();
	AddPartToList<UGPC_SI_Shotgun2>();
	AddPartToList<UGPC_SI_Shotgun3>();
	AddPartToList<UGPC_SI_Shotgun4>();
	AddPartToList<UGPC_SI_Shotgun5>();
	AddPartToList<UGPC_SI_Sniper1>();
	AddPartToList<UGPC_SI_Sniper2>();
	AddPartToList<UGPC_SI_Sniper3>();
	AddPartToList<UGPC_SI_Sniper4>();
	AddPartToList<UGPC_SI_Sniper5>();
	AddPartToList<UGPC_SI_Sniper6>();
	AddPartToList<UGPC_SI_Sniper7>();
	AddPartToList<UGPC_SI_Sniper8>();
}

TPair<TArray<UClass*>*, TArray<float>*> AProjectTRGameModeBase::SelectPartsList(const EGunPartType PartType, const int PartTier)
{
	if (PartType == EGunPartType::EGT_Receiver)
	{
		switch (PartTier)
		{
		case 1:
			return { &ReceiverClasses_T1, &ReceiverWeights_T1 };
		case 2:
			return { &ReceiverClasses_T2, &ReceiverWeights_T2 };
		case 3:
			return { &ReceiverClasses_T3, &ReceiverWeights_T3 };
		}
	}
	else if (PartType == EGunPartType::EGT_Barrel)
	{
		switch (PartTier)
		{
		case 1:
			return { &BarrelClasses_T1, &BarrelWeights_T1 };
		case 2:
			return { &BarrelClasses_T2, &BarrelWeights_T2 };
		case 3:
			return { &BarrelClasses_T3, &BarrelWeights_T3 };
		}
	}
	else if (PartType == EGunPartType::EGT_Grip)
	{
		switch (PartTier)
		{
		case 1:
			return { &GripClasses_T1, &GripWeights_T1 };
		case 2:
			return { &GripClasses_T2, &GripWeights_T2 };
		case 3:
			return { &GripClasses_T3, &GripWeights_T3 };
		}
	}
	else if (PartType == EGunPartType::EGT_Magazine)
	{
		switch (PartTier)
		{
		case 1:
			return { &MagazineClasses_T1, &MagazineWeights_T1 };
		case 2:
			return { &MagazineClasses_T2, &MagazineWeights_T2 };
		case 3:
			return { &MagazineClasses_T3, &MagazineWeights_T3 };
		}
	}
	else if (PartType == EGunPartType::EGT_Muzzle)
	{
		switch (PartTier)
		{
		case 1:
			return { &MuzzleClasses_T1, &MuzzleWeights_T1 };
		case 2:
			return { &MuzzleClasses_T2, &MuzzleWeights_T2 };
		case 3:
			return { &MuzzleClasses_T3, &MuzzleWeights_T3 };
		}
	}
	else if (PartType == EGunPartType::EGT_Sight)
	{
		switch (PartTier)
		{
		case 1:
			return { &SightClasses_T1, &SightWeights_T1 };
		case 2:
			return { &SightClasses_T2, &SightWeights_T2 };
		case 3:
			return { &SightClasses_T3, &SightWeights_T3 };
		}
	}
	else if (PartType == EGunPartType::EGT_Stock)
	{
		switch (PartTier)
		{
		case 1:
			return { &StockClasses_T1, &StockWeights_T1 };
		case 2:
			return { &StockClasses_T2, &StockWeights_T2 };
		case 3:
			return { &StockClasses_T3, &SightWeights_T3 };
		}
	}
	UE_LOG(LogTemp, Error, TEXT("SelectPartsList - Invalid argument %d %d, returning empty dummy array!"), PartType, PartTier);
	TArray<UClass*> DummyArr1;
	TArray<float> DummyArr2;
	return { &DummyArr1, &DummyArr2 };
}

ATRSpectatorPawn* AProjectTRGameModeBase::SpawnSpectator(UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params)
{
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	ATRSpectatorPawn* SpawnedPawn = Cast<ATRSpectatorPawn>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, this->SpectatorClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	// 생성자 이후 추가 로직

	SpawnedPawn = Cast<ATRSpectatorPawn>(UGameplayStatics::FinishSpawningActor(SpawnedPawn, SpawnTransform));
	return SpawnedPawn;
}

ABotCharacter* AProjectTRGameModeBase::SpawnBot(TSubclassOf<ABotCharacter> BotClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params)
{
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	ABotCharacter* SpawnedBot = Cast<ABotCharacter>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, BotClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	// 생성자 이후 추가 로직

	SpawnedBot = Cast<ABotCharacter>(UGameplayStatics::FinishSpawningActor(SpawnedBot, SpawnTransform));
	return SpawnedBot;
}

ADungeonActor* AProjectTRGameModeBase::SpawnDungeonActor(TSubclassOf<class ADungeonActor> ActorClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params)
{
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	ADungeonActor* SpawnedActor = Cast<ADungeonActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, ActorClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	// 생성자 이후 추가 로직
	// NOTE: 정규 던전 생성 과정을 통해서 생성되는 것이 아니므로, 플래그를 변경해주어야 정상적인 초기화가 진행됨
	SpawnedActor->bForceInitDuringBeginPlay = true;

	SpawnedActor = Cast<ADungeonActor>(UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform));
	return SpawnedActor;
}

ATRToken* AProjectTRGameModeBase::SpawnToken(TSubclassOf<class ATRToken> TokenClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params, int32 TokenTier)
{
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	ATRToken* SpawnedToken = Cast<ATRToken>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, TokenClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	// 생성자 이후 추가 로직
	SpawnedToken->Initialize(TokenTier);

	SpawnedToken = Cast<ATRToken>(UGameplayStatics::FinishSpawningActor(SpawnedToken, SpawnTransform));
	return SpawnedToken;
}

AIconStageActor* AProjectTRGameModeBase::SpawnIconStage(UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params)
{
	UWorld* TRWorld = GetWorld();
	if (!TRWorld || !IconStageClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnIconStage - Something went wrong!"));
		return nullptr;
	}

	// 스테이지 액터 생성
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	AIconStageActor* SpawnedStage = Cast<AIconStageActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, IconStageClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	SpawnedStage = Cast<AIconStageActor>(UGameplayStatics::FinishSpawningActor(SpawnedStage, SpawnTransform));
	return SpawnedStage;
}

ATRExplosion* AProjectTRGameModeBase::SpawnExplosion(TSubclassOf<ATRExplosion> ExplClass, UWorld* World, FVector Location, FRotator Rotation, FVector CollisionNormal, FActorSpawnParameters Params, const FExplosionInfo& NewExplInfo, bool bOverrideExplInfo)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnExplosion - Something went wrong!"));
		return nullptr;
	}

	// 오버라이드 할 정보가 주어진 경우 주어진 클래스를 무시하고 빈 템플릿을 기준으로 스테이터스를 교체한다
	TSubclassOf<ATRExplosion> SpawnClass = ExplClass;
	if (bOverrideExplInfo)
	{
		SpawnClass = ATRExplosion::StaticClass();
	}

	if (!SpawnClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnExplosion - SpawnClass is invalid!"));
		return nullptr;
	}

	// 폭발 액터 생성
	FTransform SpawnTransform(Rotation, Location, FVector(1, 1, 1));
	ATRExplosion* SpawnedExpl = Cast<ATRExplosion>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, SpawnClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	
	if (SpawnedExpl)
	{
		if (bOverrideExplInfo)
		{
			SpawnedExpl->SetExplosionInfo(NewExplInfo);
		}
		if (Params.Instigator)
		{
			SpawnedExpl->SetInstigator(Params.Instigator);
		}
		SpawnedExpl->CollisionNormal = CollisionNormal;
	}
	
	SpawnedExpl = Cast<ATRExplosion>(UGameplayStatics::FinishSpawningActor(SpawnedExpl, SpawnTransform));
	return SpawnedExpl;
}

void AProjectTRGameModeBase::ChangeGameLevel(FString LevelURL, int32 NewDepth)
{
	UWorld* CurrentWorld = GetWorld();
	UTRGameInstance* TRInst = GetTRGameInstance();
	if (CurrentWorld && TRInst)
	{
		TR_PRINT("Changing Game Level!");
		const TArray<ATRPlayerController*>& PlayerArray = GetPlayersConnected();
		TRInst->Server_CachePlayersInstanceData(PlayerArray);
		TRInst->Server_OnDescendingDungeon(NewDepth);

		APlayerController* const PC = TRInst->GetFirstLocalPlayerController();
		if (PC)
		{
			PC->ServerPause();
		}

		CurrentWorld->ServerTravel(LevelURL);
	}
}

void AProjectTRGameModeBase::InitDungeonDepth(int32 NewDepth)
{
	DungeonDepth = NewDepth;
	UpdatePlayersDungeonDepth();
}

void AProjectTRGameModeBase::UpdatePlayersDungeonDepth()
{
	const TArray<ATRPlayerController*>& ConnectedPlayers = GetPlayersConnected();
	for (ATRPlayerController* PlayerController : ConnectedPlayers)
	{
		PlayerController->Server_SetCurrDungeonDepth(DungeonDepth);
	}
}

FString AProjectTRGameModeBase::GetLevelNameOfDepth(int32 TargetDepth)
{
	if (TargetDepth < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("GetLevelNameOfDepth - Unexpected dungeon depth %d! Using default dungeon level."), TargetDepth);
		return TEXT(LVL_DUNGEON_ASSET);
	}
	else
	{
		FString* LevelName = nullptr;
		LevelName = DungeonLevels.Find(TargetDepth);
		if (!LevelName)
		{
			return TEXT(LVL_DUNGEON_ASSET);
		}
		return FString(*LevelName);
	}
	UE_LOG(LogTemp, Error, TEXT("GetLevelNameOfDepth - Unexpected error for dungeon depth %d!"), TargetDepth);
	return TEXT(LVL_DUNGEON_ASSET);
}

TArray<ATRPlayerController*> AProjectTRGameModeBase::GetPlayersConnected()
{
	TArray<ATRPlayerController*> PlayerControllers;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ATRPlayerController* PlayerController = Cast<ATRPlayerController>(Iterator->Get());
		if (PlayerController)
		{
			PlayerControllers.Add(PlayerController);
		}
	}
	return PlayerControllers;
}

TArray<AFPSCharacter*> AProjectTRGameModeBase::GetInGamePlayerCharacters(bool bIgnoreDead)
{
	TArray<AFPSCharacter*> PlayerChars;
	TArray<ATRPlayerController*> Players = GetPlayersConnected();

	for (ATRPlayerController* Player : Players)
	{
		if (IsValid(Player))
		{
			// out 상태의 컨트롤러 무시
			if (Player->GetPlayerState<ATRPlayerState>()->GetIsOut()) continue;

			// 캐릭터 필터링
			AFPSCharacter* PlayerChar = Cast<AFPSCharacter>(Player->GetPawn());
			if (PlayerChar)
			{
				if (bIgnoreDead && PlayerChar->GetHasDied())
				{
					continue;
				}
				PlayerChars.Add(PlayerChar);
			}
		}
	}
	return PlayerChars;
}

void AProjectTRGameModeBase::RespawnPlayer(ATRPlayerController* Controller, FTransform RespawnTransform, TSubclassOf<AFPSCharacter> CharacterClass, FGameCharacterInstanceData InstanceData, bool bRegenHealthAndLife)
{
	if (!IsValid(Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("RespawnPlayer() - Invalid argument(s)!"));
		return;
	}
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("RespawnPlayer() - Invalid world!"));
		return;
	}

	// 커스텀 폰 스폰 및 Possess
	TSubclassOf<APawn> CharClassToSpawn = CharacterClass;
	if (!IsValid(CharClassToSpawn)) CharClassToSpawn = DefaultPawnClass;

	AFPSCharacter* SpawnPawn = Cast<AFPSCharacter>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, CharClassToSpawn, RespawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
	if (!IsValid(SpawnPawn)) return;

	// 체력 및 생명 재생 시 인스턴스 데이터 변경
	if (bRegenHealthAndLife)
	{
		InstanceData.bHasDiedCached = true;
		InstanceData.bHasDied = false;

		int32 NewHealth = 1; // 부활 시 체력 0은 유효하지 않은 값
		if (InstanceData.bPureMaxHealthCached)
		{
			NewHealth = InstanceData.PureMaxHealth;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("RespawnPlayer - Max health is not cached! Using default value for respawned character's health."))
		}

		InstanceData.bHealthPointCached = true;
		InstanceData.HealthPoint = NewHealth;
	}

	// 인스턴스 데이터 적용
	SpawnPawn->Server_RestoreFromInstanceData(InstanceData);
	SpawnPawn = Cast<AFPSCharacter>(UGameplayStatics::FinishSpawningActor(SpawnPawn, RespawnTransform));

	// Possession change
	APawn* OriginPawn = Controller->GetPawnOrSpectator();
	if (OriginPawn)
	{
		Controller->UnPossess();
		OriginPawn->Destroy(); // 기존 폰 파괴
	}
	Controller->Possess(SpawnPawn);

	// 엔진 단 RestartPlayer
	// 이미 폰이 possess 된 상태이기 때문에 새 폰을 생성하지는 않는다
	RestartPlayerAtTransform(Controller, RespawnTransform);

	// 스폰 후 possess했기 때문에 bIsOut false로 설정
	ATRPlayerState* State = Controller->GetPlayerState<ATRPlayerState>();
	if (State)
	{
		State->SetIsOut(false);
	}
	return;
}

void AProjectTRGameModeBase::UpdateGameOverStatus()
{
	ATRGameState* CurrGameState = GetGameState<ATRGameState>();
	if (!CurrGameState)
	{
		UE_LOG(LogTemp, Error, TEXT("IsGameOver() - Wrong game state!"));
		return;
	}

	for (APlayerState* PlayerState : CurrGameState->PlayerArray)
	{
		ATRPlayerState* CurrPlayerState = Cast<ATRPlayerState>(PlayerState);
		if (CurrPlayerState && !CurrPlayerState->GetIsOut())
		{
			bIsGameOver = false;
			return;
		}
	}
	bIsGameOver = true;

	if (bIsGameOver) OnGameOver();
	return;
}

void AProjectTRGameModeBase::OnGameOver()
{
	if (!bIsGameOver)
	{
		UE_LOG(LogTemp, Error, TEXT("OnGameOver() - should not be called when the game is not over!"));
		return;
	}
	// TODO
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Game Over!"));
}

void AProjectTRGameModeBase::UpdateGameClearStatus()
{
	// TODO
}

void AProjectTRGameModeBase::OnGameClear()
{
	if (!bIsGameClear)
	{
		UE_LOG(LogTemp, Error, TEXT("OnGameClear() - should not be called when the game is not cleared!"));
		return;
	}
	// TODO
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Game Cleared!"));
}

void AProjectTRGameModeBase::UpdateDungeonTime(float DeltaTime)
{
	// 시간 초과 시 더이상 시간을 갱신하지 않음
	if (bIsTimeOver) return;

	DungeonTime += DeltaTime;
	ATRGameState* TRGS = GetGameState<ATRGameState>();
	int32 DungeonTimeSeconds = GetSecondsLeft();
	if (TRGS && TRGS->Local_GetDungeonTimeLeft() != DungeonTimeSeconds)
	{
		TRGS->Server_SetDungeonTimeLeft(DungeonTimeSeconds);
	}

	// 시간 초과
	if (DungeonTimeSeconds <= 0)
	{
		bIsTimeOver = true;
		OnTimeOver();
	}
}

void AProjectTRGameModeBase::OnTimeOver()
{
	// 레드모드 진입
	EnterRedMode();

	// TEMP
	// 전체 알림
	TArray<ATRPlayerController*> TRPlayers = GetPlayersConnected();
	for (ATRPlayerController* TRPlayer : TRPlayers)
	{
		if (IsValid(TRPlayer))
		{
			FString AlertText = FString::Printf(TEXT("The dungeon has awakened to your presence. Escape while you still can!"));
			TRPlayer->Local_AlertTextRPC(AlertText, 5.0f);
		}
	}
}

void AProjectTRGameModeBase::EnterRedMode()
{
	CurrEnemyGenStateTargetTime = -1;
	SetEnemyGenState(EEnemyGenerationState::EGS_RED);
	SetIntensity(MAX_INTENSITY);

	OnEnterRedMode();
}

void AProjectTRGameModeBase::OnEnterRedMode()
{
	ATRGameState* TRGS = GetGameState<ATRGameState>();
	if (TRGS)
	{
		TRGS->Server_ProcessRedModeEnter();
	}

	// TODO: 필요 시 추가 로직 작성
}

void AProjectTRGameModeBase::ChangeEnemyGeneration(float IntensityGoal, float ReachTime)
{
	SetTargetIntensity(IntensityGoal);

	// 시간 정보 새로 설정
	CurrEnemyGenStateTime = 0.0f;
	CurrEnemyGenStateTargetTime = ReachTime;

	// 시간에 기반해 델타 인텐시티 설정
	if (ReachTime <= 0.0f)
	{
		// 도달 시간 0 이하일 경우 곧바로 해당 타깃 값으로 설정
		SetDeltaIntensityPerSec(0.0f);
		SetIntensity(IntensityGoal);
	}
	else
	{
		SetDeltaIntensityPerSec((IntensityGoal - Intensity) / ReachTime);
	}

	// 변경된 인텐시티 정보를 기반으로 스테이트 리프레시
	RefreshEnemyGenSM();
}

void AProjectTRGameModeBase::OnBotInvalidated(ACharacter* RemoveCharacter)
{
	ABotCharacter* Removed = Cast<ABotCharacter>(RemoveCharacter);
	if (Removed)
	{
		// NOTE: 동일 캐릭터에 대해 Invalidation 요구가 중복해서 발생할 수 있다
		// (사망 이후 래그돌 상태에서 오쿨루전 컬링되는 경우 등)
		// 이 경우에도 정상적으로 처리되게 하기 위해 반드시 Remove가 하나 이상 되었는지 확인해야 한다
		if (SpawnedBots.Remove(Removed) > 0)
		{
			CurrSpawnedCount--;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OnBotInvalidated - Invalid character."));
		return;
	}

	TR_PRINT("OnBotInvalidated Delegate called!");
}

void AProjectTRGameModeBase::SetIntensity(float Value)
{
	Intensity = FMath::Clamp(Value, MIN_INTENSITY, MAX_INTENSITY);
}

void AProjectTRGameModeBase::SetTargetIntensity(float Value)
{
	TargetIntensity = FMath::Clamp(Value, MIN_INTENSITY, MAX_INTENSITY);
}

void AProjectTRGameModeBase::SetDeltaIntensityPerSec(float Value)
{
	DeltaIntensityPerSec = Value;
}

void AProjectTRGameModeBase::FindAndBindDungeonGenerator()
{
	TArray<AActor*> Generator;
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATRDungeonGenerator::StaticClass(), Generator);
	}
	if (Generator.Num() <= 0)
	{
		return;
	}
	else if (Generator.Num() > 1)
	{
		UE_LOG(LogTemp, Error, TEXT("FindAndBindDungeonGenerator - There should only be one dungeon generator within a level!"));
	}

	ATRDungeonGenerator* Target = Cast<ATRDungeonGenerator>(Generator[0]);
	if (!Target)
	{
		UE_LOG(LogTemp, Error, TEXT("FindAndBindDungeonGenerator - Unexpected error!"));
		return;
	}
	this->DungeonGenerator = Target;
}

void AProjectTRGameModeBase::DungeonTick(float DeltaTime)
{
	// 던전 경과 시간 갱신
	UpdateDungeonTime(DeltaTime);

	// 시간은 틱 델타 단위로 더해짐
	CurrEnemyGenStateTime += DeltaTime;

	UpdateIntensity(DeltaTime);
	if (CurrEnemyGenStateTargetTime <= CurrEnemyGenStateTime)
	{
		// 기본 사이클을 따라서 다음 번 스테이트로 넘어간다
		ChangeToNextState();
	}
	ManageEnemies(DeltaTime);
}

void AProjectTRGameModeBase::UpdateIntensity(float DeltaTime)
{
	SetIntensity(Intensity + (DeltaIntensityPerSec * DeltaTime));
}

void AProjectTRGameModeBase::RefreshEnemyGenSM()
{
	if (EnemyGenState == EEnemyGenerationState::EGS_RED)
	{
		return;
	}

	// 부동소수점 연산 과정에서의 약간의 오차를 허용
	if (FMath::Abs(TargetIntensity - Intensity) <= UE_KINDA_SMALL_NUMBER)
	{
		Intensity = TargetIntensity; // 오차 보정
		if (Intensity >= MAX_INTENSITY)
		{
			SetEnemyGenState(EEnemyGenerationState::EGS_PEAK);
		}
		else if (Intensity <= MIN_INTENSITY)
		{
			SetEnemyGenState(EEnemyGenerationState::EGS_RELAX);
		}
		else
		{
			SetEnemyGenState(EEnemyGenerationState::EGS_IDLE);
		}
	}
	else if (Intensity < TargetIntensity)
	{
		switch (EnemyGenState)
		{
		case EEnemyGenerationState::EGS_RISE:
			break;
		case EEnemyGenerationState::EGS_IDLE:
		case EEnemyGenerationState::EGS_FALL:
		case EEnemyGenerationState::EGS_RELAX:
			SetEnemyGenState(EEnemyGenerationState::EGS_RISE);
			break;
		case EEnemyGenerationState::EGS_PEAK:
			UE_LOG(LogTemp, Warning, TEXT("UpdateEnemyGenSM - Unexpected behaviour. Either the Intensity has changed unexpectedly(you should try change TargetIntensity instead), or TargetIntensity has been set pass the maximum limit."));
			break;
		}
	}
	else if (Intensity > TargetIntensity)
	{
		switch (EnemyGenState)
		{
		case EEnemyGenerationState::EGS_FALL:
			break;
		case EEnemyGenerationState::EGS_IDLE:
		case EEnemyGenerationState::EGS_RISE:
		case EEnemyGenerationState::EGS_PEAK:
			SetEnemyGenState(EEnemyGenerationState::EGS_FALL);
			break;
		case EEnemyGenerationState::EGS_RELAX:
			UE_LOG(LogTemp, Warning, TEXT("UpdateEnemyGenSM - Unexpected behaviour. Either the Intensity has changed unexpectedly(you should try change TargetIntensity instead), or TargetIntensity has been set below the minimum limit."));
			break;
		}
	}
}

void AProjectTRGameModeBase::ChangeToNextState()
{
	// 현재 스테이트가 종료되고 다른 스테이트로 변하고 있기 때문에 현재 스테이트 유지시간은 0으로 리셋
	CurrEnemyGenStateTime = 0.0f;

	switch (EnemyGenState)
	{
	default:
	case EEnemyGenerationState::EGS_RELAX:
	case EEnemyGenerationState::EGS_IDLE: // IDLE 상태에서 다시 기본 사이클로 복귀할 경우 무조건 RISE 시킴
		// RISE로 이동
		ChangeEnemyGeneration(MAX_INTENSITY, FMath::RandRange(RiseTimeRange.X, RiseTimeRange.Y));
		break;
	case EEnemyGenerationState::EGS_RISE:
		// PEAK로 이동
		ChangeEnemyGeneration(MAX_INTENSITY, FMath::RandRange(PeakTimeRange.X, PeakTimeRange.Y));
		break;
	case EEnemyGenerationState::EGS_PEAK:
		// FALL로 이동
		ChangeEnemyGeneration(MIN_INTENSITY, FMath::RandRange(FallTimeRange.X, FallTimeRange.Y));
		break;
	case EEnemyGenerationState::EGS_FALL:
		// RELAX로 이동
		ChangeEnemyGeneration(MIN_INTENSITY, FMath::RandRange(RelaxTimeRange.X, RelaxTimeRange.Y));
		break;
	case EEnemyGenerationState::EGS_RED:
		// 더 이상 다른 스테이트로 이동하지 않고 반복
		break;
	}
}

void AProjectTRGameModeBase::SetEnemyGenState(EEnemyGenerationState NewState)
{
	EnemyGenState = NewState;
}

void AProjectTRGameModeBase::ManageEnemies(float DeltaTime)
{
	////////////TESTING
	//UE_LOG(LogTemp, Error, TEXT("%f %f CurCnt:%d TarCnt:%d"), Intensity, TargetIntensity, CurrSpawnedCount, GetCurrDesiredEnemyCount());

	if (!DungeonGenerator) return;
	const uint32 SpawnInnerDepth = DungeonGenerator->Local_GetOcculusionDepth();
	const uint32 SpawnOuterDepth = SpawnInnerDepth + SpawnDepthOffset;

	// 웨이브 주기가 되었는지 확인
	TimeSinceLastSpawnWave += DeltaTime;
	TimeSinceLastDespawnWave += DeltaTime;

	int32 SpawnPendingCount = GetCurrDesiredEnemyCount() - CurrSpawnedCount;

	// 스폰 웨이브
	if (SpawnPendingCount > 0 && TimeSinceLastSpawnWave > GetCurrSpawnWaveCycle())
	{
		TR_PRINT("wave - spawn");

		// 내곽 바깥, 외곽 안
		TPair<TSet<URoom*>, TSet<URoom*>> OcculusionResult = DungeonGenerator->Server_GetRoomsOfOcculusionDepthInBetween(SpawnInnerDepth, SpawnOuterDepth);
		const TArray<URoom*>& SpawnRoomArray = OcculusionResult.Get<0>().Array();

		if (!SpawnRoomArray.IsEmpty())
		{
			TSubclassOf<ABotCharacter> SpawnBotClass = GetRandomSpawnMonsterClass(DungeonDepth);
			if (!SpawnBotClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("ManageEnemies - Selected spawn bot class is invalid. Please check the game mode property."));
				return;
			}

			URoom* SpawnRoom = SpawnRoomArray[FMath::Rand() % SpawnRoomArray.Num()];
			const TArray<ASpawnPoint*>& SpawnPoints = *DungeonGenerator->RoomSpawnPoints.Find(SpawnRoom);
			FVector SpawnLoc;
			bool bSpawnLocFound = TRUtils::GetRandomSpawnLocation(SpawnPoints, ESpawnPointType::SPT_ENEMY_REGULAR, SpawnLoc);
			if (!bSpawnLocFound)
			{
				UE_LOG(LogTemp, Error, TEXT("ManageEnemies - SpawnPoint Not found! This could be a normal behaviour."));

				// 다음 재시도까지 약간의 텀을 가진다
				TimeSinceLastSpawnWave = MinSpawnWaveCycle;
				return;
			}

			const FVector& SpawnOffset = { 0.0f, 0.0f, 50.0f };
			ABotCharacter* SpawnedBot = SpawnBot(SpawnBotClass, GetWorld(), SpawnLoc + SpawnOffset, FRotator(0, 0, 0), FActorSpawnParameters());
			if (SpawnedBot)
			{
				TR_PRINT("SPAWN!");

				// 스폰
				CurrSpawnedCount++;
				SpawnedBots.Add(SpawnedBot);
				SpawnedBot->Server_OnDeathDelegate.AddDynamic(this, &AProjectTRGameModeBase::OnBotInvalidated);

				// 컨트롤러 풀에서 꺼내옴
				AIPool->Animate(SpawnedBot);

				// 외곽 안이므로 스폰은 가능하지만 내곽 바깥이므로 디액티베이트 상태로 스폰한다
				SpawnedBot->Deactivate();
			}
		}

		// 스폰 쿨타임 리셋
		TimeSinceLastSpawnWave = 0.0f;
	}

	// 디스폰 웨이브
	// 단일 틱에 부하가 과해지는 것을 막기 위해 스폰 웨이브와 동시 실행을 else if로 방지
	// 디스폰 및 디액티베이션을 처리
	else if (TimeSinceLastDespawnWave > DESPAWN_WAVE_DELAY)
	{
		TR_PRINT("wave - despawn");
		// 디스폰 웨이브
		TimeSinceLastDespawnWave = 0.0f;

		// 외곽 영역 내의 방들만 유효함
		const TSet<URoom*>& ValidRooms = DungeonGenerator->Server_GetRoomsOfOcculusionDepth(SpawnOuterDepth);
		TSet<ABotCharacter*> SpawnedBotsSnapshot = SpawnedBots;
		TSet<ABotCharacter*> RefreshedBots;
		RefreshedBots.Empty();
		int32 CurrActiveBotCnt = 0;

		for (ABotCharacter* SpawnedBot : SpawnedBotsSnapshot/*Invalidation 과정에서 컨테이너 항목이 변경되므로 캐시 사용이 필수적임*/)
		{
			bool bIsBotValid = false;
			FBox BotBox = SpawnedBot->GetCapsuleComponent()->Bounds.GetBox();
			if (!IsValid(SpawnedBot)) continue;
			// 유효한 방 안에 있는지 확인
			for (URoom* ValidRoom : ValidRooms)
			{
				if (ValidRoom->GetBounds().GetBox().Intersect(BotBox))
				{
					bIsBotValid = true;
					break;
				}
			}

			if (!bIsBotValid)
			{
				TR_PRINT("DESPAWN!");

				// 컨트롤러 풀로 반환
				AIPool->Inanimate(SpawnedBot);

				// 봇 무효화
				OnBotInvalidated(SpawnedBot);

				// 디스폰
				DespawnBot(SpawnedBot);
			}
			else
			{
				RefreshedBots.Add(SpawnedBot);
				if (SpawnedBot->GetBotActive())
				{
					// 현 시점의 액티브 봇의 개수이기 때문에 Refresh 된 리스트에 포함되는 액터들만 대상으로 계산한다
					CurrActiveBotCnt++;
				}
			}
		}
		SpawnedBots = RefreshedBots;
		int32 ActiveCntLeft = FMath::RoundToInt32(MinActiveBotRatio * CurrSpawnedCount) - CurrActiveBotCnt;

		// 내곽 안의 방에 있는 액터들 액티베이트
		const TSet<URoom*>& ActiveRooms = DungeonGenerator->Server_GetRoomsOfOcculusionDepth(SpawnInnerDepth);
		for (ABotCharacter* SpawnedBot : SpawnedBots)
		{
			bool bBotIsInInner = false;
			FBox BotBox = SpawnedBot->GetCapsuleComponent()->Bounds.GetBox();
			if (!IsValid(SpawnedBot)) continue;
			// 내곽 안에 있는지 확인
			for (URoom* ActiveRoom : ActiveRooms)
			{
				if (ActiveRoom->GetBounds().GetBox().Intersect(BotBox))
				{
					bBotIsInInner = true;
					break;
				}
			}

			if (bBotIsInInner)
			{
				if (!SpawnedBot->GetBotActive())
				{
					// 내곽 액터는 항상 활성화를 시켜야 하므로 무조건 활성화를 처리한다
					SpawnedBot->Activate();
					ActiveCntLeft--;
				}
			}
			else
			{
				// 내곽이 아닌 액터들은 ActiveCntLeft가 양수인 경우 활성화 시킨다 (혹은 유지시킨다)
				if (ActiveCntLeft > 0)
				{
					if (!SpawnedBot->GetBotActive()) SpawnedBot->Activate();
					ActiveCntLeft--;
				}
				else
				{
					if (SpawnedBot->GetBotActive())
					{
						SpawnedBot->Deactivate();
					}
				}
			}

			// NOTE: TSet의 순회 순서는 랜덤하게 변할 수 있기 때문에 
			// 어쩔 수 없이 외곽 봇들이 Activate 되었다 Deactivate 되었다를 반복하는 현상이 발생한다
		}
	}
}

FORCEINLINE int32 AProjectTRGameModeBase::GetCurrDesiredEnemyCount() const
{
	return (int32)FMath::Lerp(MinEnemyCount, MaxEnemyCount, Intensity); // Intensity 정비례
}

float AProjectTRGameModeBase::GetCurrSpawnWaveCycle() const
{
	return FMath::Lerp(MaxSpawnWaveCycle, MinSpawnWaveCycle, Intensity); // Intensity 반비례
}

TSubclassOf<class ABotCharacter> AProjectTRGameModeBase::GetRandomSpawnMonsterClass(int32 SpawnDepth)
{
	// TODO : 현재 상황 고려한 몬스터 선택
	return TRUtils::SelectSpawnMonster(SpawnableRegularBotClasses);
}

bool AProjectTRGameModeBase::DespawnBot(ABotCharacter* Bot)
{
	if (!Bot || !Bot->GetCanDespawn()) return false;
	Bot->Destroy();
	return true;
}

void AProjectTRGameModeBase::UpdateIconOf(UInvObject* InvObj)
{
	if (!InvObj)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateIconOf - Something went wrong!"));
		return;
	}

	// 기존 아이콘 생성에 사용한 액터 존재 시 파괴
	AIconStageActor* PrevStageActor = InvObj->GetCurrIconStageActor();
	if (PrevStageActor)
	{
		PrevStageActor->Destroy();
	}

	FVector SpawnLoc = IssueNewSpawnLocation();
	FActorSpawnParameters StageSpawnParam;
	StageSpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	FActorSpawnParameters ItemSpawnParam;
	ItemSpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AIconStageActor* StageActor = SpawnIconStage(GetWorld(), SpawnLoc, BaseSpawnRotation, StageSpawnParam);
	if (StageActor && StageActor->DisplayComponent)
	{
		// 이어지는 Setup단계에서 적절한 트랜스폼으로 재설정되므로 여기서 사용되는 위치, 회전은 크게 의미가 없음
		ABaseItem* SpawnedItem = SpawnDecorativeItem(InvObj, GetWorld(), StageActor->DisplayComponent->GetComponentLocation(), StageActor->DisplayComponent->GetComponentRotation(), ItemSpawnParam);
		if (SpawnedItem)
		{
			// 아이콘 생성 시 아이템이 굉장히 먼 거리에 생성되는데, 이때 레플리케이션이 되어야 클라이언트 사이드에도 오브젝트 메쉬가 캡처 가능해짐
			SpawnedItem->NetCullDistanceSquared = MAX_FLT;

			// 아이템 별로 추가 회전이 필요할 경우 같이 전달한다
			StageActor->SetupDisplayActor(SpawnedItem, SpawnedItem->IconDisplayRotation);

			// 모든 과정이 성공적으로 처리된 경우 바인딩
			InvObj->SetCurrIconStageActor(StageActor);
		}
	}
}

FVector AProjectTRGameModeBase::IssueNewSpawnLocation()
{
	IconTransformIssueCount += 1;
	int32 ColNum = IconTransformIssueCount % IconsPerRow;
	int32 RowNum = IconTransformIssueCount / IconsPerRow;
	return BaseSpawnLocation + (SpawnLocatonHorizontalDelta * ColNum) + (SpawnLocatonVerticalDelta * RowNum);
}

void AProjectTRGameModeBase::BossfightTick(float DeltaTime)
{
	if (!HasClearedBossfight())
	{
		UpdateBossfightState(DeltaTime);
		ManageBosses(DeltaTime);
	}
}

void AProjectTRGameModeBase::UpdateBossfightState(float DeltaTime)
{
	if (bBossfightCleared)
	{
		// 보스전을 클리어한 이후 다시 클리어 실패가 되는 경우는 발생하지 않음
		UE_LOG(LogTemp, Warning, TEXT("UpdateBossfightState - Called unnecessarily after clearing the bossfight."));
		return;
	}
	if (bMustSlayBossToClear && bBossSlain)
	{
		ClearBossfight();
	}
	// TODO: 추가 클리어 조건 추가 (e.g. 맵의 특정 지역에 도착 등)
}

bool AProjectTRGameModeBase::HasClearedBossfight() const
{
	return bBossfightCleared;
}

void AProjectTRGameModeBase::ManageBosses(float DeltaTime)
{
	// 모든 보스들이 생성이 완료된 후에는 더이상 변경할 게 없음
	if (bBossGenerated) return;

	TArray<TSubclassOf<ABotCharacter>> SpawnBossClasses = GetRandomSpawnBossClasses(DungeonDepth);
	if (SpawnBossClasses.IsEmpty() || SpawnBossClasses.Contains(nullptr))
	{
		UE_LOG(LogTemp, Error, TEXT("ManageBosses - One or more boss class is invalid! Aborting the bossfight!"));
		ClearBossfight();
		return;
	}

	URoom* SpawnRoom = BossRoom;
	if (!SpawnRoom)
	{
		UE_LOG(LogTemp, Error, TEXT("ManageBosses - Boss room is invallid! Aborting the bossfight!"));
		ClearBossfight();
		return;
	}

	const TArray<ASpawnPoint*>& AllSpawnPoints = *DungeonGenerator->RoomSpawnPoints.Find(SpawnRoom);
	TArray<ASpawnPoint*> SpawnPoints;
	for (ASpawnPoint* SpawnPoint : AllSpawnPoints)
	{
		if (SpawnPoint->SpawnType == ESpawnPointType::SPT_ENEMY_BOSS) SpawnPoints.Add(SpawnPoint);
	}

	if (SpawnPoints.Num() < SpawnBossClasses.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("ManageBosses - Boss spawn point number is insufficient! Bosses should not use the same spawn point to prevent weird placements. Aborting bossfight!"));
		ClearBossfight();
		return;
	}

	for (int32 Idx = 0; Idx < SpawnBossClasses.Num(); ++Idx)
	{
		TSubclassOf<ABotCharacter> SpawnBossClass = SpawnBossClasses[Idx];
		ASpawnPoint* CurrSpawnPoint = SpawnPoints[Idx]; // 쓰이지 않고 남는 스폰 포인트들이 있을 수 있음
		if (!CurrSpawnPoint)
		{
			UE_LOG(LogTemp, Error, TEXT("ManageBosses - The chosen spawn point is invalid! Will abort if nothing has spawned before."));
			if (SpawnedBosses.IsEmpty())
			{
				ClearBossfight();
				return;
			}
			else
			{
				// 이미 소환된 대상이 있다면 완전히 Abort하는 대신 소환된 대상들만 남긴 채로 보스전을 진행함
				continue;
			}
		}

		FVector SpawnLoc = CurrSpawnPoint->GetRandomSpawnLocation();
		const FVector& SpawnOffset = { 0.0f, 0.0f, 50.0f };
		ABotCharacter* SpawnedBoss = SpawnBot(SpawnBossClass, GetWorld(), SpawnLoc + SpawnOffset, FRotator(0, 0, 0), FActorSpawnParameters());
		if (SpawnedBoss)
		{
			TR_PRINT("BOSS SPAWN!");

			// 스폰
			SpawnedBosses.Add(SpawnedBoss);
			SpawnedBoss->Server_OnDeathDelegate.AddDynamic(this, &AProjectTRGameModeBase::OnSpawnedBossDeath);

			// NOTE: 보스는 항상 활성화된 상태를 유지하며, 컨트롤러 풀을 사용하지 않고 처음 할당된 컨트롤러를 고정적으로 사용한다
			SpawnedBoss->Activate();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ManageBosses - Failed to spawn boss actor! Will abort if nothing has spawned before."));
			if (SpawnedBosses.IsEmpty())
			{
				ClearBossfight();
				return;
			}
			else
			{
				continue;
			}
		}
	}

	bBossGenerated = true;
}

void AProjectTRGameModeBase::OnSpawnedBossDeath(ACharacter* DeadCharacter)
{
	ABotCharacter* Removed = Cast<ABotCharacter>(DeadCharacter);
	if (Removed)
	{
		SpawnedBosses.Remove(Removed);
	}

	if (SpawnedBosses.IsEmpty())
	{
		bBossSlain = true;
	}

	////TESTING
	TR_PRINT("OnSpawnedBossDeath Delegate called!");
}

void AProjectTRGameModeBase::OnBossfightCleared()
{
	TR_PRINT("Bossfight Cleared!");

	ABossDungeonGenerator* BossDG = Cast<ABossDungeonGenerator>(DungeonGenerator);
	if (BossDG)
	{
		BossDG->OnBossFightCleared();
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("OnBossfightCleared - Fatal gameplay error; Cannot access BossDungeonGenerator!"));
	}
	return;
}

void AProjectTRGameModeBase::ClearBossfight()
{
	bBossfightCleared = true;
	OnBossfightCleared();
}

TArray<TSubclassOf<ABotCharacter>> AProjectTRGameModeBase::GetRandomSpawnBossClasses(int32 SpawnDepth)
{
	// TODO : 현재 상황 고려한 몬스터 선택
	// TODO : 보스몹을 하나 이상 선택할 수 있도록 수정
	// TODO : 보스몹을 하나 이상 소환할 때는 특정 조합이 항상 유지되야 하므로 랜덤 선택 시 특정 짝끼리만 매칭되도록 구현
	TArray<TSubclassOf<class ABotCharacter>> Result;
	TSubclassOf<ABotCharacter> Chosen = TRUtils::SelectSpawnMonster(SpawnableBossClasses);
	if (Chosen)
	{
		Result.Add(Chosen);
	}
	return Result;
}