// Copyright (C) 2024 by Haguk Kim

#include "FPSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "InputConfig.h"
#include "AnimConfig.h"
#include "TRGameInstance.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "BaseProjectile.h"
#include "BaseCharacterMovementComponent.h"
#include "FirstPersonMeshComponent.h"
#include "FPSCameraComponent.h"
#include "TRMacros.h"
#include "TRSpectatorPawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Blueprint/UserWidget.h"
#include "InventoryComponent.h"
#include "EquipSystem.h"
#include "BaseItem.h"
#include "WieldItem.h"
#include "GunItem.h"
#include "TRSoul.h"
#include "ActItemComponent.h"
#include "TRPlayerController.h"
#include "RecoilAnimationComponent.h"
#include "TRPlayerState.h"
#include "TRCameraShake.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WidgetComponent.h"
#include "PlayerNameTagWidget.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/DamageType.h"
#include "ProjectTRGameModeBase.h"
#include "TRDungeonGenerator.h"
#include "TRGameState.h"
#include "RoomLevel.h"
#include "Room.h"
#include "TRShop.h"
#include "FPSHUD.h"
#include "InvBasedWidget.h"
#include "TRHUDWidget.h"


////////////TESTING
#include "TRChest.h"

// Sets default values
AFPSCharacter::AFPSCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

#pragma region /** Networking */
	bReplicates = true;
	NetCullDistanceSquared = MAX_FLT;
#pragma endregion

#pragma region /** Component Initialization */
	// 메쉬
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	check(SkeletalMesh != nullptr);

	// 플레이어 캐릭터의 경우 항상 본을 계산한다
	// 총기 격발 등의 경우 총구 위치가 본에 Dependent하기 때문
	SkeletalMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// 무브먼트
	GetTRCharacterMovementComponent()->bCanSlide = true;

	// 일인칭 카메라
	FPSCamera = CreateDefaultSubobject<UFPSCameraComponent>(TEXT("FPSCamera"));
	check(FPSCamera != nullptr);
	FPSCamera->SetupAttachment(SkeletalMesh, TEXT("head")); // BeginPlay에서 다시 소켓에 재부착함; 여기서는 BP 뷰포트에서의 편집을 위해 임시로 부착함
	FPSCamera->bUsePawnControlRotation = true;

	// 삼인칭 카메라와 스프링암
	TPSCamera = CreateDefaultSubobject<UFPSCameraComponent>(TEXT("TPSCamera"));
	check(TPSCamera != nullptr);

	TPSSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TPSSpringArm"));
	check(TPSSpringArm != nullptr);

	TPSSpringArm->SetupAttachment(GetCapsuleComponent());
	TPSCamera->SetupAttachment(TPSSpringArm);
	TPSSpringArm->bUsePawnControlRotation = true;

	// 시야 설정
	Local_SetCameraViewPerspective(false); // 1인칭으로 시작

	// 경험치
	ExpComp = CreateDefaultSubobject<UExpComponent>(TEXT("ExpComponent"));
	check(ExpComp != nullptr);
	ExpComp->ResetAllExp();
	ExpComp->SetIsReplicated(true);

	// 위젯
	NameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidgetComponent"));
	check(NameWidgetComponent != nullptr);
	NameWidgetComponent->SetupAttachment(RootComponent);
	NameWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NameWidgetComponent->SetSimulatePhysics(false);
	NameWidgetComponent->SetEnableGravity(false);
	NameWidgetComponent->SetVisibility(false); // false로 시작
#pragma endregion

#pragma region /** Variable Defaults */
	// 룸 옵저버 사용
	bShouldActivateRoomObserver = true;

	// 사망 시 플레이어 액터는 조금 더 오랜 시간 남는다
	DestructionTimeDefault = 120.0f;
#pragma endregion

#pragma region /** Debug */
#pragma endregion
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 소켓에 어태치
	FAttachmentTransformRules CamAttachRule = FAttachmentTransformRules::SnapToTargetIncludingScale;
	CamAttachRule.bWeldSimulatedBodies = true;
	FPSCamera->AttachToComponent(GetMesh(), CamAttachRule, TEXT("FPSCam"));

	// 기타 정보 초기화
	AimedTargetUICollisionParams.AddIgnoredActor(GetOwner());
	AimedTargetUICollisionParams.AddIgnoredActor(this);
	AimedTargetUIObjQueryParams.AddObjectTypesToQuery(ECC_PlayerPawn); // 필요 시 추가
}

void AFPSCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 연결된 모든 위젯 제거
	UWorld* World = GetWorld();
	if (World)
	{
		ATRPlayerController* PC = World->GetFirstPlayerController<ATRPlayerController>();
		if (PC)
		{
			PC->Local_DerefPawnBoundedWidgets(this);
		}
	}
}

void AFPSCharacter::Local_OnPlayerDamageInflictedToTarget_Implementation(AGameCharacter* Target, FVector DamageLocation, int32 Damage, bool bIsKillshot, bool bIsCrit)
{
	// NOTE: Target 사망 시 Invalid 할 수 있다, 타깃은 아군이나 자신일 수 있다
	UWorld* World = GetWorld();
	if (World)
	{
		ATRGameState* TRGS = World->GetGameState<ATRGameState>();
		if (TRGS)
		{
			TRGS->Local_DisplayDamageNumber(GetWorld(), TRGS->DefaultDamageNumberWidgetClass, FTransform(DamageLocation), Damage, false);
		}
	}

	if (Damage > 0)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController)
		{
			// 크로스헤어 이펙트
			AFPSHUD* FPSHUD = Cast<AFPSHUD>(PlayerController->GetHUD());
			if (FPSHUD && FPSHUD->Crosshair)
			{
				FPSHUD->Crosshair->PlayHitEffect(bIsKillshot, bIsCrit);
			}

			// TODO: 히트사운드
			// TODO: 피아식별 후 이펙트 다르게 처리
		}
	}
}

void AFPSCharacter::Move(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		const FRotator MovementRotation(0, Controller->GetControlRotation().Yaw, 0);

		GetTRCharacterMovementComponent()->SetMovingForward(MoveValue.Y);
		GetTRCharacterMovementComponent()->SetMovingRight(MoveValue.X);

		if (MoveValue.Y != 0.f)
		{
			const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(Direction, MoveValue.Y);
		}

		if (MoveValue.X != 0.f)
		{
			const FVector Direction = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(Direction, MoveValue.X);
		}
	}
}

void AFPSCharacter::StartSprint(const FInputActionValue& Value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Sprinting!"));
	GetTRCharacterMovementComponent()->OnInput_SprintStart();
}

void AFPSCharacter::StopSprint(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Stop sprinting!"));
	GetTRCharacterMovementComponent()->OnInput_SprintStop();
}

void AFPSCharacter::Look(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		// TODO: 마우스 감도 설정
		const FVector2D LookValue = Value.Get<FVector2D>() * RotationSensitivity;
		
		// 애니메이션 회전 속도는 마우스 회전 속도와 일치
		GetTRCharacterMovementComponent()->SetTurnRate(FMath::Clamp(LookValue.X, -1.0f, 1.0f));

		if (LookValue.X != 0.f)
		{
			AddControllerYawInput(LookValue.X);
		}

		if (LookValue.Y != 0.f)
		{
			AddControllerPitchInput(LookValue.Y * -1.0f);
		}
	}
}

void AFPSCharacter::ApplyJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Jumped!"));
		Jump();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Stop Jumping!"));
		StopJumping();
	}
}

void AFPSCharacter::Attack(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Attacked!"));
	if (Host_CanPerformFire(true))
	{
		Host_RegisterFire(true);
	}
}

void AFPSCharacter::Attack2(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Attacked! (2)"));

	//////////////////////
	/////TESTING
	ATRPlayerController* TRController = Cast<ATRPlayerController>(GetController());
	if (TRController)
	{
		TRController->Local_AlertTextRPC(TEXT("ATTACK2 Widget test"), 10.0f);
	}

	//////TESTING 2
	if (HasAuthority())
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(GetWorld()->GetAuthGameMode());
		if (TRGM)
		{
			if (TRGM->DungeonGenerator)
			{
				TSet<URoom*> Rooms = TRGM->DungeonGenerator->Server_GetRoomsOfOcculusionDepth(TRGM->DungeonGenerator->Local_GetOcculusionDepth());
				for (auto r : Rooms)
				{
					TR_PRINT_FSTRING("%s", *(r->GetName()));
				}
			}

			///////TESTING 3
			TRGM->ChangeEnemyGeneration(1.0f, 5.0f);
		}
	}

	////////////////////TESTING 4
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("RESISTANCE: %f, %f, %f"), GetStat_PhysicalResistance(), GetStat_ElementalResistance(), GetStat_MagicalResistance(), HasAuthority());
	}

	//////////////TESTING5
	const TArray<UStatusEffect*>& TEMP = GetAppliedStatEffects();
	UE_LOG(LogTemp, Error, TEXT("STATEFFCOUNT: %d, AUTH %d"), TEMP.Num(), HasAuthority());
	///////////////////

	////////////////TESTING7
	if (HasAuthority())
	{
		for (TActorIterator<ATRChest> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (*ActorItr)
			{
				TR_PRINT("TESTING: Chest activation");
				(*ActorItr)->Server_TryOpenChest();
			}
		}
	}
	///////////////////////


	if (Host_CanPerformFire(false))
	{
		Host_RegisterFire(false);
		Local_PlayItemFx(false);
	}
}

void AFPSCharacter::AttackStop(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Attack stopped!"));
	Server_RegisterStopFireRPC(true);
	Local_StopItemFx(true);
}

void AFPSCharacter::Attack2Stop(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Attack stopped! (2)"));
	Server_RegisterStopFireRPC(false);
	Local_StopItemFx(false);
}

void AFPSCharacter::Duck(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Ducking!"));

		GetTRCharacterMovementComponent()->OnInput_CrouchStart();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Un-Ducking!"));

		GetTRCharacterMovementComponent()->OnInput_CrouchStop();
	}
}

void AFPSCharacter::Slide(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Sliding!"));
	// TESTING TODO FIXME
	if (HasAuthority())
	{
		AProjectTRGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AProjectTRGameModeBase>();
		if (GameMode)
		{
			GameMode->SpawnRandomizedGunItem(GetWorld(), GetHandPointInfo().Get<0>(), FRotator(), FActorSpawnParameters(), 0);
		}
	}
}

void AFPSCharacter::Taunt(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Taunting!"));
	UBaseCharacterMovementComponent* MoveComp = GetTRCharacterMovementComponent();

	// TODO
}

void AFPSCharacter::Roll(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Rolling!"));
	UBaseCharacterMovementComponent* MoveComp = GetTRCharacterMovementComponent();

	// 클라이언트의 로컬 인풋 방향을 전달
	MoveComp->Server_RegisterRollRPC(MoveComp->GetMovingForward(), MoveComp->GetMovingRight());
}

void AFPSCharacter::Interact(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Interact!"));
	Server_InteractRPC();
}

void AFPSCharacter::AccessInventory(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return; // 로컬 액션

	ATRPlayerController* PC = Cast<ATRPlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		UUserWidget* NewWidget = PC->Local_CreateWidget(PC->InvWidgetClass, this);
		UInvBasedWidget* InvWidget = Cast<UInvBasedWidget>(NewWidget);
		if (InvWidget)
		{
			// 초기화
			InvWidget->InvComp = InvComponent;
			InvWidget->EquSys = EquipSystem;

			PC->Local_DisplayWidget(InvWidget, WZO_INV);
			PC->Local_FocusWidget(InvWidget, true, true);
		}
		else if (NewWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("AccessInventory - InvWidget has wrong widget type!"));
			PC->Local_DerefWidget(NewWidget);
		}
	}
}

void AFPSCharacter::SwitchTo0(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Switch input 0!"));
	SwitchWeaponSlotTo(0);
}

void AFPSCharacter::SwitchTo1(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Switch input 1!"));
	SwitchWeaponSlotTo(1);
}

void AFPSCharacter::SwitchTo2(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Switch input 2!"));
	SwitchWeaponSlotTo(2);
}

void AFPSCharacter::SwitchTo3(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Switch input 3!"));
	SwitchWeaponSlotTo(3);
}

void AFPSCharacter::ToggleViewPerspective(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Changing perspective!"));
	Server_ToggleCameraViewPerspective();
}

void AFPSCharacter::Server_RegisterShopPurchase_Implementation(ATRShop* ShopActor, UInvObject* TradeItem)
{
	ShopActor->Server_PurchaseFromShop(this, TradeItem);
}

void AFPSCharacter::Server_RegisterShopSell_Implementation(ATRShop* ShopActor, UInvObject* TradeItem)
{
	ShopActor->Server_SellToShop(this, TradeItem);
}

void AFPSCharacter::GetActorEyesViewPoint(FVector& out_Location, FRotator& out_Rotation) const
{
	UCameraComponent* CameraComp = Host_GetCurrViewCamera();
	if (CameraComp)
	{
		out_Location = CameraComp->GetComponentLocation();
		out_Rotation = CameraComp->GetComponentRotation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetActorEyesViewPoint - Invalid camera! Using physical eyes instead."));
		APawn::GetActorEyesViewPoint(out_Location, out_Rotation);
	}
	return;
}

TPair<FVector, FRotator> AFPSCharacter::GetHandPointInfo()
{
	FVector Location;
	FRotator Rotation;

	// 물리적 눈 위치와 카메라 회전 방향을 사용한다
	FRotator UnusedParamA;
	FVector UnusedParamB;
	APawn::GetActorEyesViewPoint(Location, UnusedParamA);
	GetActorEyesViewPoint(UnusedParamB, Rotation);

	Location += Rotation.Vector() * GrabOffset;
	return TPair<FVector, FRotator>(Location, Rotation);
}

TPair<FVector, FRotator> AFPSCharacter::GetMuzzleInfo()
{
	// 우선 카메라 시야 위치를 가져온다
	FVector Location;
	FRotator Rotation;
	GetActorEyesViewPoint(Location, Rotation);

	// 지향 방향으로 Offset만큼 이동
	Location += Rotation.Vector() * MuzzleOffset;
	return TPair<FVector, FRotator>(Location, Rotation);
}

void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FGameCharacterInstanceData AFPSCharacter::Server_GetInstanceData()
{
	FGameCharacterInstanceData NewData = Super::Server_GetInstanceData();
	if (ExpComp)
	{
		NewData.Experience = ExpComp->GetCurrTotalExp();
		NewData.bExperienceCached = true;

		NewData.LevelExperience = ExpComp->GetCurrLevelExp();
		NewData.bLevelExperienceCached = true;

		NewData.Level = ExpComp->GetLevel();
		NewData.bLevelCached = true;
	}
	return NewData;
}

bool AFPSCharacter::Server_RestoreFromInstanceData(const FGameCharacterInstanceData& InstData)
{
	bool bSuccess = true;
	bSuccess &= Super::Server_RestoreFromInstanceData(InstData);

	if (ExpComp)
	{
		if (InstData.bExperienceCached)
		{
			ExpComp->SetCurrTotalExp(InstData.Experience);
		}
		if (InstData.bLevelExperienceCached)
		{
			ExpComp->SetCurrLevelExp(InstData.LevelExperience);
		}
		if (InstData.bLevelCached)
		{
			ExpComp->SetLevel(InstData.Level);
		}
	}

	return bSuccess;
}

void AFPSCharacter::Server_OnDamageInflictedToTarget(AGameCharacter* Target, FDamageEvent const& DamageEvent, int32 Damage, bool bIsKillshot, bool bIsCrit)
{
	Super::Server_OnDamageInflictedToTarget(Target, DamageEvent, Damage, bIsKillshot, bIsCrit);

	// 서버 로직
	// 공격자에게만 보이는 데미지 넘버 위젯 액터 소환
	TArray<ATRPlayerController*> TargetViewers;
	ATRPlayerController* PC = GetController<ATRPlayerController>();
	if (PC) TargetViewers.Add(PC);
	FHitResult DamageHitRes;
	FVector DamageImpulseDirection;
	DamageEvent.GetBestHitInfo(Target, this, DamageHitRes, DamageImpulseDirection);

	// 공격자 로컬 로직 처리 요청
	this->Local_OnPlayerDamageInflictedToTarget(Target, DamageHitRes.Location, Damage, bIsKillshot, bIsCrit);
}

UFPSCameraComponent* AFPSCharacter::Host_GetCurrViewCamera() const
{
	bool bTPSCamActive = TPSCamera->IsActive();
	bool bFPSCamActive = FPSCamera->IsActive();
	if (bHost_IsThirdPersonView && bTPSCamActive) return TPSCamera;
	else if (!bHost_IsThirdPersonView && bFPSCamActive) return FPSCamera;

	UE_LOG(LogTemp, Error, TEXT("Host_GetCurrViewCamera - View perspective and camera activeness is out of sync!"));
	if (bFPSCamActive) return FPSCamera;
	if (bTPSCamActive) return TPSCamera;
	UE_LOG(LogTemp, Error, TEXT("Host_GetCurrViewCamera - Both FPS and TPS Camera is inactive!"));
	return FPSCamera;
}

void AFPSCharacter::Local_PlayCameraShake(TSubclassOf<UTRCameraShake> CamShake, float Scale)
{
	UWorld* World = GetWorld();
	if (CamShake && World)
	{
		APlayerController* LocalController = World->GetFirstPlayerController();
		if (LocalController)
		{
			LocalController->ClientStartCameraShake(static_cast<TSubclassOf<UCameraShakeBase>>(CamShake), Scale);
			return;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Local_PlayCameraShake - Something went wrong!"));
}

void AFPSCharacter::Server_ToggleCameraViewPerspective_Implementation()
{
	if (!HasAuthority()) return;
	Multicast_SetCameraViewPerspective(!bHost_IsThirdPersonView); // 토글
}

void AFPSCharacter::Multicast_SetCameraViewPerspective_Implementation(bool bIsThirdPerson)
{
	Local_SetCameraViewPerspective(bIsThirdPerson);
}

void AFPSCharacter::Local_SetCameraViewPerspective(bool bIsThirdPerson)
{
	bHost_IsThirdPersonView = bIsThirdPerson;
	if (bIsThirdPerson)
	{
		FPSCamera->SetActive(false);
		TPSCamera->SetActive(true);
		TPSSpringArm->SetActive(true);
	}
	else
	{
		FPSCamera->SetActive(true);
		TPSCamera->SetActive(false);
		TPSSpringArm->SetActive(false);
	}
}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 인풋 맵핑 등록
	AddLocalPlayerInputMappingContext(InputMapping, 0/*TODO: Enum화*/, true);

	// 인풋 액션 바인딩
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputConfig->MoveInputAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Move);
		EnhancedInputComponent->BindAction(InputConfig->SprintAction, ETriggerEvent::Triggered, this, &AFPSCharacter::StartSprint);
		EnhancedInputComponent->BindAction(InputConfig->SprintAction, ETriggerEvent::Completed, this, &AFPSCharacter::StopSprint);
		EnhancedInputComponent->BindAction(InputConfig->LookInputAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Look);
		EnhancedInputComponent->BindAction(InputConfig->JumpInputAction, ETriggerEvent::Triggered, this, &AFPSCharacter::ApplyJump);
		EnhancedInputComponent->BindAction(InputConfig->AttackAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Attack);
		EnhancedInputComponent->BindAction(InputConfig->Attack2Action, ETriggerEvent::Triggered, this, &AFPSCharacter::Attack2);
		EnhancedInputComponent->BindAction(InputConfig->AttackStopAction, ETriggerEvent::Triggered, this, &AFPSCharacter::AttackStop);
		EnhancedInputComponent->BindAction(InputConfig->Attack2StopAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Attack2Stop);
		EnhancedInputComponent->BindAction(InputConfig->InteractAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Interact);
		EnhancedInputComponent->BindAction(InputConfig->InventoryAction, ETriggerEvent::Triggered, this, &AFPSCharacter::AccessInventory);
		EnhancedInputComponent->BindAction(InputConfig->DuckAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Duck);
		EnhancedInputComponent->BindAction(InputConfig->SlideAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Slide);
		EnhancedInputComponent->BindAction(InputConfig->TauntAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Taunt);
		EnhancedInputComponent->BindAction(InputConfig->RollAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Roll);
		EnhancedInputComponent->BindAction(InputConfig->SwitchTo0Action, ETriggerEvent::Triggered, this, &AFPSCharacter::SwitchTo0);
		EnhancedInputComponent->BindAction(InputConfig->SwitchTo1Action, ETriggerEvent::Triggered, this, &AFPSCharacter::SwitchTo1);
		EnhancedInputComponent->BindAction(InputConfig->SwitchTo2Action, ETriggerEvent::Triggered, this, &AFPSCharacter::SwitchTo2);
		EnhancedInputComponent->BindAction(InputConfig->SwitchTo3Action, ETriggerEvent::Triggered, this, &AFPSCharacter::SwitchTo3);
		EnhancedInputComponent->BindAction(InputConfig->ToggleViewPerspectiveAction, ETriggerEvent::Triggered, this, &AFPSCharacter::ToggleViewPerspective);
	}
}

void AFPSCharacter::AddLocalPlayerInputMappingContext(const UInputMappingContext* Context, int32 Priority, bool bClearAllMappings)
{
	// 인풋 맵핑 여러 개 등록 시 하나의 인풋에 대해 우선순위 높은 순으로 확인하며 처리한다
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (Context)
			{
				if (bClearAllMappings) InputSystem->ClearAllMappings();
				InputSystem->AddMappingContext(Context, Priority);
			}
		}
	}
	return;
}

TArray<FHitResult> AFPSCharacter::Reach()
{
	FVector StartLocation;
	FRotator Rotation;
	GetActorEyesViewPoint(StartLocation, Rotation);
	UWorld* World = GetWorld();

	if (World)
	{
		// 상호작용 거리 제한
		FVector EndLocation = StartLocation + (Rotation.Vector() * ReachDistance);

		// 라인 트레이싱
		FCollisionQueryParams TraceParams(FName(TEXT("LineTrace")), true, this);
		TraceParams.bTraceComplex = true;
		TraceParams.bReturnPhysicalMaterial = false;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(GetOwner());
		FCollisionObjectQueryParams ObjColParam;
		ObjColParam.AddObjectTypesToQuery(ECC_Interactive);
		TArray<FHitResult> InteractiveHits;
		if (World->LineTraceMultiByObjectType(InteractiveHits, StartLocation, EndLocation, ObjColParam, CollisionParams))
		{
			//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 2.0f);

			////////TESTING
			for (const FHitResult& Res : InteractiveHits)
			{
				if (Res.GetActor())
				{
					TR_PRINT_FSTRING("%s", *((Res.GetActor())->GetName()));
				}
			}




			return InteractiveHits;
		}
	}
	return TArray<FHitResult>();
}

ABaseItem* AFPSCharacter::ReachItem()
{
	const TArray<FHitResult>& Hits = Reach();
	if (!Hits.IsEmpty())
	{
		for (int32 Idx = Hits.Num() - 1; Idx >= 0; --Idx)
		{
			ABaseItem* ReachedItem = Cast<ABaseItem>(Hits[Idx].GetActor());
			if (ReachedItem)
			{
				return ReachedItem;
			}
		}
	}
	return nullptr;
}

void AFPSCharacter::Server_ProcessDeath()
{
	Super::Server_ProcessDeath();

	// 사망 직후에도 이름 위젯이 남아있는 것은 부자연스러움
	NameWidgetComponent->DestroyComponent();

	Server_SpawnSoulAt(GetActorLocation(), GetActorRotation());
	Server_StartSpectating();
}

void AFPSCharacter::Multicast_ProcessDeath()
{
	Super::Multicast_ProcessDeath();

	// 로컬 로직
	// 사망 시 캐릭터와 연결된 모든 위젯 삭제
	// 현재로썬 캐릭터 사망 후에도 해당 캐릭터에 대한 위젯을 유지해야 하는 상황은 존재하지 않음
	UWorld* World = GetWorld();
	if (World)
	{
		// 캐릭터에 할당된 컨트롤러는 이미 Unpossess된 상태이므로 직접 로컬 컨트롤러를 가져온다
		ATRPlayerController* PC = World->GetFirstPlayerController<ATRPlayerController>();
		if (PC && PC->IsLocalController())
		{
			PC->Local_DerefPawnBoundedWidgets(this);
		}
	}

	// 사망 시 모든 드래그 드랍 취소
	UTRGameInstance* TRGI = GetGameInstance<UTRGameInstance>();
	if (TRGI) TRGI->Local_CancelAllDragDrops();
}

void AFPSCharacter::Server_SpawnSoulAt(FVector Location, FRotator Rotation)
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* GameMode = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (GameMode)
		{
			if (!IsValid(DropSoulItemClass))
			{
				UE_LOG(LogTemp, Error, TEXT("Server_SpawnSoulAt - %s's soul item class is not set! Aborting."), *(GetName()));
				return;
			}

			// 데이터가 하나라도 문제가 있으면 영혼 소환 실패
			TSubclassOf<ABaseItem> SoulClass = DropSoulItemClass;
			TSubclassOf<AFPSCharacter> CharClass = GetClass();
			ATRPlayerController* TRController = Cast<ATRPlayerController>(this->GetController());
			FGameCharacterInstanceData CharData = Server_GetInstanceData();

			if (!SoulClass || !CharClass || !TRController)
			{
				UE_LOG(LogTemp, Error, TEXT("Server_SpawnSoulAt - Player Character Data is not valid! %d %d %d, Aborting."), (SoulClass != nullptr), (CharClass != nullptr), (TRController != nullptr));
				return;
			}

			ATRSoul* Soul = Cast<ATRSoul>(GameMode->SpawnItem(SoulClass, World, Location, Rotation, FActorSpawnParameters()));
			if (!IsValid(Soul))
			{
				UE_LOG(LogTemp, Error, TEXT("Server_SpawnSoulAt - Something went wrong during soul spawning!"));
				return;
			}

			Soul->Server_SetCharacterClass(CharClass);
			Soul->Server_SetController(TRController);
			Soul->Server_SetInstanceData(CharData);
		}
	}
}

float AFPSCharacter::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ReturnValue = Super::TakeDamage(DamageTaken, DamageEvent, EventInstigator, DamageCauser);

	// 데미지 피격 폰을 보유한 호스트에 대해 시각 효과 처리
	FHitResult DamageHit;
	FVector DamageImpulseDirection;
	DamageEvent.GetBestHitInfo(this, DamageCauser, DamageHit, DamageImpulseDirection);

	// NOTE: 데미지의 Source 위치를 TraceStart를 사용해 저장한다
	// 이때 TakeDamage에 전달되는 TraceStart 값은 데미지의 위치를 표현하기 위한 값으로 실제 트레이싱에 사용된 시작지점과 다를 수 있다
	Local_OnDamageTaken(ReturnValue, DamageHit.TraceStart);
	return ReturnValue;
}

void AFPSCharacter::Local_OnDamageTaken_Implementation(float Damage, FVector DamageLocation)
{
	if (Damage > 0)
	{
		if (this->IsLocallyViewed())
		{
			Local_PlayCameraShake(CamShakeConfig->OnDamageCamShake, 6.0f/* TODO: 수치에 비례하게 */);
		}

		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController)
		{
			// 크로스헤어 이펙트
			AFPSHUD* FPSHUD = Cast<AFPSHUD>(PlayerController->GetHUD());
			if (FPSHUD && FPSHUD->Crosshair)
			{
				FPSHUD->Crosshair->DrawHitCircle(DamageLocation, this, Damage);
			}
		}
	}
}

void AFPSCharacter::Server_OnLevelUp(int32 NewLevel)
{
	// 레벨업 시 보유한 모든 총기들의 탄약 회복
	Server_RefillAllAmmo();
	
	// 레벨업에 성공한 클라이언트의 로컬에서 로직 수행
	Client_OnLevelUpRPC(NewLevel);
}

void AFPSCharacter::Client_OnLevelUpRPC_Implementation(int32 NewLevel)
{
	ATRPlayerController* TRController = Cast<ATRPlayerController>(GetController());
	if (TRController)
	{
		FString LevelUpText = FString::Printf(TEXT("Level up to %d! Ammo refilled"), NewLevel);
		TRController->Local_AlertTextRPC(LevelUpText, 5.0f);
	}
}

void AFPSCharacter::Server_OnRoomEnter(ARoomLevel* RoomLevel)
{
	Super::Server_OnRoomEnter(RoomLevel);
}

void AFPSCharacter::Multicast_SetNameTagText_Implementation(const FString& NewName)
{
	if (NameWidgetComponent)
	{
		UPlayerNameTagWidget* TagWidget = Cast<UPlayerNameTagWidget>(NameWidgetComponent->GetWidget());
		if (TagWidget)
		{
			TagWidget->SetName(NewName);
		}
	}
}

void AFPSCharacter::Client_RequestUpdateNames_Implementation()
{
	if (!HasAuthority()) return;
	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (TRGM)
		{
			TRGM->UpdatePlayerNames();
		}
	}
}

void AFPSCharacter::Local_SetAimedTargetUITracking(bool bActivate)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Local_SetAimedTargetUITracking - Invalid world!"));
		return;
	}

	if (bActivate)
	{
		World->GetTimerManager().SetTimer(
			AimedTargetUITimer,
			this,
			&AFPSCharacter::Local_UpdateAimedTargetUI,
			Local_AimedTargetUIUpdateRate,
			true
		);
	}
	else
	{
		World->GetTimerManager().ClearTimer(AimedTargetUITimer);
	}
}

void AFPSCharacter::Local_SetNameWidgetVisibility(bool bVisible)
{
	NameWidgetComponent->SetVisibility(bVisible);
}

void AFPSCharacter::Local_PreCreateWidgets()
{
	if (IsLocallyControlled())
	{
		ATRPlayerController* PC = Cast<ATRPlayerController>(GetController());
		if (PC)
		{
			// HUD
			UUserWidget* NewWidget = PC->Local_CreateWidget(PC->HUDWidgetClass, this);
			UTRHUDWidget* HUDWidget = Cast<UTRHUDWidget>(NewWidget);
			if (HUDWidget)
			{
				// 초기화
				HUDWidget->SetTarget(this);

				PC->Local_DisplayWidget(HUDWidget, WZO_HUD);
				// 포커싱 불필요
			}
			else if (NewWidget)
			{
				UE_LOG(LogTemp, Error, TEXT("Local_PreCreateWidgets - HUDWidget has wrong widget type!"));
				PC->Local_DerefWidget(NewWidget);
			}
		}
	}
}

FString AFPSCharacter::Host_GetCurrWeaponAmmoLeft() const
{
	if (EquipSystem)
	{
		AGunItem* CurrGun = Cast<AGunItem>(EquipSystem->GetCurrWeaponActor());
		if (IsValid(CurrGun))
		{
			return FString::Printf(TEXT("%d"), CurrGun->Host_GetCurrAmmo());
		}
	}
	return FString("");
}

void AFPSCharacter::Local_UpdateAimedTargetUI()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 라인 트레이싱
	FHitResult AimedUIHit;
	FVector StartLocation;
	FRotator Rotation;
	GetActorEyesViewPoint(StartLocation, Rotation);
	FVector EndLocation = StartLocation + (Rotation.Vector() * ReachDistance);

	bool bHit = World->LineTraceSingleByObjectType(AimedUIHit, StartLocation, EndLocation, AimedTargetUIObjQueryParams, AimedTargetUICollisionParams);

	AActor* CurrActor = AimedUIHit.GetActor();
	AActor* PrevActor = PrevAimedActor.Get();
	if (CurrActor == PrevActor)
	{
		PrevActor = nullptr;
	}

	Local_UpdateAimedTarget(PrevActor, false);
	Local_UpdateAimedTarget(CurrActor, true);
	PrevAimedActor = CurrActor;
}

void AFPSCharacter::Local_UpdateAimedTarget(AActor* Target, bool bVisibility)
{
	if (!Target) return;

	AFPSCharacter* TargetActor = Cast<AFPSCharacter>(Target);
	if (TargetActor)
	{
		TargetActor->Local_SetNameWidgetVisibility(bVisibility);
		return;
	}
	// TODO: 필요 시 추가
}

void AFPSCharacter::Server_StartSpectating()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this->GetController();

	UWorld* World = GetWorld();
	AProjectTRGameModeBase* GameMode = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_StartSpectating - unable to get appropriate GameMode."));
		return;
	}

	// 관전 상태 진입 시 컨트롤러 Out 처리
	ATRPlayerState* TRPlayerState = GetPlayerState<ATRPlayerState>();
	if (TRPlayerState)
	{
		TRPlayerState->SetIsOut(true);
	}

	// 관전 시작
	ATRSpectatorPawn* SpecPawn = GameMode->SpawnSpectator(World, GetActorLocation(), GetActorRotation(), SpawnParams);

	GetController()->Possess(SpecPawn); // 내부적으로 UnPossess 호출하니 별도 호출 필요 X
	SpecPawn->Server_SetOriginalCharacterData(this);
	SpecPawn->Server_ChangeSpecTargetRPC(true);
}

void AFPSCharacter::ProcessBeforeItemPickup(ABaseItem* ReachedItem)
{
	Super::ProcessBeforeItemPickup(ReachedItem);
}
