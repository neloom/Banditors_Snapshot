// Copyright (C) 2024 by Haguk Kim


#include "TRSpectatorPawn.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "ProjectTRGameModeBase.h"
#include "TRPlayerController.h"
#include "TRPlayerState.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputConfig.h"
#include "Components/SphereComponent.h"

ATRSpectatorPawn::ATRSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bHighPriority = false;
	SetActorEnableCollision(false);
}

void ATRSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FBox ATRSpectatorPawn::GetComponentsBoundingBox(bool bNonColliding, bool bIncludeFromChildActors) const
{
	// 관전 폰의 바운딩 박스는 NonColliding 플래그가 true여야 함;
	// 관전 폰 자체가 콜리전을 처리하지 않을 수 있기 때문에 이렇게 하지 않을 경우 룸 컬링이 제대로 처리되지 않을 수 있음
	return Super::GetComponentsBoundingBox(true, false);
}

void ATRSpectatorPawn::Server_SetOriginalCharacterData(AGameCharacter* Character)
{
	AFPSCharacter* PlayerCharacter = Cast<AFPSCharacter>(Character);
	if (PlayerCharacter)
	{
		this->RespawnPlayerClass = PlayerCharacter->GetClass(); // 클래스 정보
		this->RespawnPlayerInstanceData = PlayerCharacter->Server_GetInstanceData(); // 인스턴스 정보
	}
}

void ATRSpectatorPawn::Server_ChangeSpecTargetRPC_Implementation(bool Direction)
{
	if (!HasAuthority()) return;

	ACharacter* OriginTarget = SpectatingTarget;
	ACharacter* FirstValidTarget = nullptr; // 가장 인덱스가 작은 유효 타깃
	ACharacter* LastValidTarget = nullptr; // 가장 인덱스가 큰 유효 타깃
	ACharacter* TargetNextOrigin = nullptr;
	ACharacter* TargetPrevOrigin = nullptr;
	int OriginTargetIdx = -1;

	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* GameMode = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (GameMode)
		{
			TArray<ATRPlayerController*> PlayersConnected = GameMode->GetPlayersConnected();
			if (PlayersConnected.IsEmpty())
			{
				SetSpectatingTarget(nullptr);
				return;
			}
			for (int Index = 0; Index < PlayersConnected.Num(); ++Index)
			{
				ATRPlayerController* PlayerController = PlayersConnected[Index];

				// 게임 세션에 참여중이 아닌 경우 제외
				ATRPlayerState* TRPlayerState = PlayerController->GetPlayerState<ATRPlayerState>();
				if (!IsValid(TRPlayerState) || TRPlayerState->GetIsOut()) continue;

				// 관전 폰은 무시하고, 조작 중인 폰을 가져온다
				AGameCharacter* TargetCharacter = Cast<AGameCharacter>(PlayerController->GetCharacter());
				if (IsValid(TargetCharacter))
				{
					// 헤드, 테일 추적
					if (!FirstValidTarget) FirstValidTarget = TargetCharacter;
					LastValidTarget = TargetCharacter;

					// 기존 타깃인지 확인
					if (OriginTarget == TargetCharacter)
					{
						OriginTargetIdx = Index;
					}
					else
					{
						// 타깃 위치를 아직 못찾았을 경우 타깃 이전에 있다고 간주하고 캐싱
						if (OriginTargetIdx < 0) TargetPrevOrigin = TargetCharacter;
						else if (!TargetNextOrigin) TargetNextOrigin = TargetCharacter;
					}
				}
			}
		}
	}

	if (OriginTargetIdx < 0)
	{
		SetSpectatingTarget(FirstValidTarget);
	}
	else
	{
		if (Direction)
		{
			if (TargetNextOrigin)
			{
				SetSpectatingTarget(TargetNextOrigin);
			}
			else
			{
				SetSpectatingTarget(FirstValidTarget);
			}
		}
		else
		{
			if (TargetPrevOrigin)
			{
				SetSpectatingTarget(TargetPrevOrigin);
			}
			else
			{
				SetSpectatingTarget(LastValidTarget);
			}
		}
	}

	return;
}

void ATRSpectatorPawn::OnRep_SpecTargetChange()
{
	// NOTE: OnRep는 서버에선 호출되지 않음
	OnSpecTargetChange();
}

void ATRSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 인풋 맵핑 등록
	AddLocalPlayerInputMappingContext(InputMapping, 1/*TODO: Enum화*/, true);

	// 인풋 액션 바인딩
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputConfig->AttackAction, ETriggerEvent::Triggered, this, &ATRSpectatorPawn::SpecChangeNext);
		EnhancedInputComponent->BindAction(InputConfig->Attack2Action, ETriggerEvent::Triggered, this, &ATRSpectatorPawn::SpecChangePrev);
	}
}

void ATRSpectatorPawn::AddLocalPlayerInputMappingContext(const UInputMappingContext* Context, int32 Priority, bool bClearAllMappings)
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

void ATRSpectatorPawn::SpecChangeNext(const FInputActionValue& Value)
{
	Server_ChangeSpecTargetRPC(true);
}

void ATRSpectatorPawn::SpecChangePrev(const FInputActionValue& Value)
{
	//Server_ChangeSpecTargetRPC(false);
	// TESTING TEMP
	Server_RespawnPlayerRPC();
}

void ATRSpectatorPawn::Server_RespawnPlayerRPC_Implementation()
{
	if (!HasAuthority()) return;
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_RespawnPlayerRPC() - Invalid world!"));
		return;
	}
	AProjectTRGameModeBase* GameMode = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_RespawnPlayerRPC() - Invalid game mode!"));
		return;
	}

	GameMode->RespawnPlayer(Cast<ATRPlayerController>(GetController()), GetTransform()/*TODO: FIXME*/, RespawnPlayerClass, RespawnPlayerInstanceData, true);
	return;
}

void ATRSpectatorPawn::SetSpectatingTarget(ACharacter* NewTarget)
{
	if (NewTarget != nullptr && NewTarget == SpectatingTarget) return;
	else
	{
		PrevSpectatingTarget = SpectatingTarget;
		SpectatingTarget = NewTarget;
		OnSpecTargetChange();
	}
}

void ATRSpectatorPawn::OnSpecTargetChange()
{
	if (!HasAuthority())
	{
		if (IsValid(SpectatingTarget)) return Client_OnNewSpecTargetSet();
		return Client_OnNoSpecTargetExist();
	}
	else
	{
		if (IsValid(SpectatingTarget)) return Server_OnNewSpecTargetSet();
		return Server_OnNoSpecTargetExist();
	}
}

void ATRSpectatorPawn::Server_OnNewSpecTargetSet()
{
	APlayerController* SpecController = Cast<APlayerController>(GetController());
	if (!SpecController)
	{
		UE_LOG(LogTemp, Error, TEXT("Server unable to get a valid player controller of the spectator pawn!"));
		return;
	}
	if (!IsValid(SpectatingTarget))
	{
		Server_OnNoSpecTargetExist();
		return;
	}

	SpecController->SetViewTargetWithBlend(SpectatingTarget);
	Local_OnViewTargetSet();

	// TODO: 추가 로직
	UE_LOG(LogTemp, Warning, TEXT("Server_OnNewSpecTargetSet()"));
}

void ATRSpectatorPawn::Client_OnNewSpecTargetSet()
{
	// 클라이언트의 경우 접근이 가능한 자기 자신의 컨트롤러, 즉 로컬 ViewTarget만 변경한다
	APlayerController* SpecController = Cast<APlayerController>(GetController());
	if (SpecController)
	{
		SpecController->SetViewTargetWithBlend(SpectatingTarget);
		Local_OnViewTargetSet();
	}
	
	// TODO: 추가 로직
	UE_LOG(LogTemp, Warning, TEXT("Client_OnNewSpecTargetSet()"));
}

void ATRSpectatorPawn::Server_OnNoSpecTargetExist()
{
	// TODO: 추가 로직
	UE_LOG(LogTemp, Warning, TEXT("Server_OnNoSpecTargetExist()"));
}

void ATRSpectatorPawn::Client_OnNoSpecTargetExist()
{
	// TODO: 추가 로직
	UE_LOG(LogTemp, Warning, TEXT("Client_OnNoSpecTargetExist()"));
}

void ATRSpectatorPawn::Local_OnViewTargetSet()
{
	AFPSCharacter* PrevFPSChar = Cast<AFPSCharacter>(PrevSpectatingTarget);
	AFPSCharacter* CurrFPSChar = Cast<AFPSCharacter>(SpectatingTarget);
	
	// TODO: 필요 시 추가 로직 작성
}
