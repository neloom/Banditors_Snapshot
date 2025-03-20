// Copyright (C) 2024 by Haguk Kim


#include "TRPlayerController.h"
#include "TRGameInstance.h"
#include "CustomUtil.h"
#include "TextAlertWidget.h"
#include "FPSCharacter.h"
#include "TRShop.h"
#include "ShopBasedWidget.h"
#include "TRHUDWidget.h"
#include "ProjectTRGameModeBase.h"

ATRPlayerController::ATRPlayerController()
{
}

void ATRPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// DEBUG TEMP
	//CustomUtil::SetGraphicsSettingsCustom();

	// 게임 인풋 사용하도록 설정
	SetInputMode(FInputModeGameOnly());

	// 폰 변경 시의 로직 델리게이트 연동
	if (IsLocalController())
	{
		OnPossessedPawnChanged.AddDynamic(this, &ATRPlayerController::Local_OnPawnPossessionChange);
		// 최초 1회 수동 호출
		if (APawn* ControlledPawn = GetPawn())
		{
			Local_OnPawnPossessionChange(nullptr, ControlledPawn);
		}
	}

	// 월드 깊이 저장
	if (HasAuthority() && GetWorld())
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(GetWorld()->GetAuthGameMode());
		if (TRGM)
		{
			TRGM->UpdatePlayersDungeonDepth();
		}
	}
}

void ATRPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATRPlayerController::Local_OnPawnPossessionChange(APawn* OldPosPawn, APawn* NewPosPawn)
{
	// NOTE: 서버 로직 필요 시 OnPossess 등 다른 함수 사용할 것
	if (!this->IsLocalController()) return;

	// Null일 수 있다
	if (OldPosPawn)
	{
		// NOTE: 위젯 삭제를 여기서 처리하게 될 경우 순서가 꼬여 문제가 발생한다
		// 위젯은 액터 Destruction 시 호출된다

		AFPSCharacter* OldFPSPawn = Cast<AFPSCharacter>(OldPosPawn);
		if (OldFPSPawn)
		{
			OldFPSPawn->Local_SetAimedTargetUITracking(false);
		}
	}

	if (NewPosPawn)
	{
		AFPSCharacter* NewFPSPawn = Cast<AFPSCharacter>(NewPosPawn);
		if (NewFPSPawn)
		{
			NewFPSPawn->Local_PreCreateWidgets();

			// 새 플레이어가 성공적으로 접속해 Possession을 마쳤으므로 이름 갱신을 요청
			NewFPSPawn->Client_RequestUpdateNames();

			NewFPSPawn->Local_SetAimedTargetUITracking(true);
		}
	}
}

UUserWidget* ATRPlayerController::Local_CreateWidget(TSubclassOf<UUserWidget> WidgetClass, APawn* BoundedTo)
{
	if (!IsLocalController() || !WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Local_CreateWidget - Invalid widget creation."));
		return nullptr;
	}
	
	UUserWidget* CreatedWidget = CreateWidget(this, WidgetClass);
	if (CreatedWidget)
	{
		if (BoundedTo)
		{
			if (PawnBoundWidgets.Contains(BoundedTo))
			{
				PawnBoundWidgets[BoundedTo].Add(CreatedWidget);
			}
			else
			{
				TSet<UUserWidget*> WidgetSet = TSet<UUserWidget*>();
				WidgetSet.Add(CreatedWidget);
				PawnBoundWidgets.Add(BoundedTo, WidgetSet);
			}
		}
		else
		{
			HostBoundWidgets.Add(CreatedWidget);
		}
		return CreatedWidget;
	}
	return nullptr;
}

void ATRPlayerController::Local_DisplayWidget(UUserWidget* Widget, int32 ZOrder)
{
	if (!Widget) return;
	Widget->AddToViewport(ZOrder);
}

void ATRPlayerController::Local_CollapseWidget(UUserWidget* Widget)
{
	if (!Widget) return;
	Widget->SetVisibility(ESlateVisibility::Collapsed);
}

void ATRPlayerController::Local_FocusWidget(UUserWidget* Widget, bool bShowCursor, bool bFlushKey)
{
	FInputModeUIOnly InputMode = FInputModeUIOnly();
	InputMode.SetWidgetToFocus(Widget->GetCachedWidget());
	SetInputMode(InputMode);
	if (bFlushKey) FlushPressedKeys();
	SetShowMouseCursor(bShowCursor);
}

void ATRPlayerController::Local_FocusGame(bool bShowCursor, bool bFlushKey)
{
	FInputModeGameOnly InputMode = FInputModeGameOnly();
	SetInputMode(InputMode);
	if (bFlushKey) FlushPressedKeys();
	SetShowMouseCursor(bShowCursor);
}

void ATRPlayerController::Local_DerefWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget)) return;

	// 포커스 있을 경우 해제
	if (Widget->HasUserFocus(this))
	{
		Local_FocusGame(false, true);
	}

	// 위젯 트리에서 제거
	Widget->RemoveFromParent();
	Widget->RemoveFromRoot();

	// 로컬 레퍼런스 제거
	bool bWasRelevant = false;
	for (TPair<APawn*, TSet<UUserWidget*>>& Pair : PawnBoundWidgets)
	{
		bWasRelevant |= (Pair.Value.Remove(Widget) > 0);
	}
	bWasRelevant |= (HostBoundWidgets.Remove(Widget) > 0);
	if (!bWasRelevant)
	{
		UE_LOG(LogTemp, Warning, TEXT("Local_DerefWidget - Tried to dereference a non-bound widget."));
	}
}

void ATRPlayerController::Local_DerefPawnBoundedWidgets(APawn* BoundPawn)
{
	if (!BoundPawn) return;

	// 위젯 트리에서 제거
	TSet<UUserWidget*>* WidgetsRef = PawnBoundWidgets.Find(BoundPawn);
	if (!WidgetsRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("Local_DerefPawnBoundedWidgets - Tried to dereference a non-bound pawn's widget"));
		return;
	}
	for (UUserWidget* Widget : *WidgetsRef)
	{
		// 포커스 있을 경우 해제
		if (Widget->HasUserFocus(this))
		{
			Local_FocusGame(false, true);
		}
		Widget->RemoveFromParent();
		Widget->RemoveFromRoot();
	}
	WidgetsRef->Empty();

	// 로컬 레퍼런스 제거
	PawnBoundWidgets.Remove(BoundPawn);
}

void ATRPlayerController::Local_DerefHostBoundedWidgets()
{
	for (UUserWidget* Widget : HostBoundWidgets)
	{
		Local_DerefWidget(Widget);
	}
	HostBoundWidgets.Empty();
}

void ATRPlayerController::Local_StartShoppingRPC_Implementation(ATRShop* ShopLogicActor)
{
	if (!IsLocalController()) return;
	AFPSCharacter* ShoppingCharacter = Cast<AFPSCharacter>(GetCharacter());
	if (!ShoppingCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Local_StartShoppingRPC_Implementation - The controller does not possess AFPSCharacter."));
		return;
	}
	if (!ShopLogicActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Local_StartShoppingRPC_Implementation - ShopActor is Invalid."));
		return;
	}

	UUserWidget* NewWidget = Local_CreateWidget(ShopWidgetClass, ShoppingCharacter);
	UShopBasedWidget* ShopWidget = Cast<UShopBasedWidget>(NewWidget);
	if (ShopWidget)
	{
		// 초기화
		ShopWidget->ShopActor = ShopLogicActor;

		Local_DisplayWidget(ShopWidget, WZO_SHOP);
		Local_FocusWidget(ShopWidget, true, true);
	}
}

void ATRPlayerController::Server_SetCurrDungeonDepth(int32 Depth)
{
	if (!HasAuthority())
	{
		return;
	}
	CurrDungeonDepth = Depth;

	// 서버의 경우 수동 호출
	Local_OnCurrDungeonDepthUpdated();
}

int32 ATRPlayerController::Local_GetCurrDungeonDepth()
{
	return CurrDungeonDepth;
}

void ATRPlayerController::OnRep_CurrDungeonDepth()
{
	Local_OnCurrDungeonDepthUpdated();
}

void ATRPlayerController::Local_OnCurrDungeonDepthUpdated()
{
	// UI 업데이트 (서버,클라)
	AGameCharacter* GameChar = Cast<AGameCharacter>(GetCharacter());
	if (GameChar && GameChar->Local_GetBoundHUDWidget().IsValid())
	{
		GameChar->Local_GetBoundHUDWidget()->UpdateDungeonDepth();
	}
}

void ATRPlayerController::Local_AlertTextRPC_Implementation(const FString& Text, float Duration)
{
	if (!IsLocalController()) return; // 서버에서 자기가 보유하지 않은 위젯에 대한 변경사항은 무시된다
	
	UUserWidget* CreatedWidget = Local_CreateWidget(TextAlertWidgetClass);
	UTextAlertWidget* TextAlertWidget = Cast<UTextAlertWidget>(CreatedWidget);
	if (TextAlertWidget)
	{
		// 인터랙션 비활성화
		TextAlertWidget->SetIsFocusable(false);
		TextAlertWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		Local_DisplayWidget(TextAlertWidget, WZO_ALERT);
		// 알림 위젯의 경우 포커싱은 필요하지 않음
		
		TextAlertWidget->SetTextForDuration(Text, Duration);
	}
	else if (CreatedWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Local_AlertTextRPC_Implementation - Created widget is not a valid type. Please check the bounded class type."));
		Local_DerefWidget(CreatedWidget);
	}
}
