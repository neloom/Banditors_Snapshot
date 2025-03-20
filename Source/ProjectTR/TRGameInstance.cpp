// Copyright (C) 2024 by Haguk Kim


#include "TRGameInstance.h"
#include "TRPlayerController.h"
#include "ProjectTRGameModeBase.h"
#include "GameCharacter.h"
#include "GameFramework/PlayerState.h"
#include "TRMacros.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Blueprint/DragDropOperation.h"
#include "SoundSubsystem.h"

UTRGameInstance::UTRGameInstance()
{
	OnNotifyPreClientTravel().AddUObject(this, &UTRGameInstance::Local_OnLevelTransitionBegin);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UTRGameInstance::Local_OnLevelTransitionComplete);
}

USoundSubsystem* UTRGameInstance::GetSoundManager()
{
	return GetSubsystem<USoundSubsystem>();
}

void UTRGameInstance::Server_CachePlayersInstanceData(const TArray<class ATRPlayerController*>& TRPlayers)
{
	Server_CachedPlayersInstanceData.Reset();
	for (ATRPlayerController* TRPlayer : TRPlayers)
	{
		AGameCharacter* GamePawn = Cast<AGameCharacter>(TRPlayer->GetPawn());
		if (GamePawn && TRPlayer->PlayerState)
		{
			FString PlayerId = TRPlayer->PlayerState->GetUniqueId().ToString();
			TR_PRINT_FSTRING("cache %s", *PlayerId);
			Server_CachedPlayersInstanceData.Add(PlayerId, GamePawn->Server_GetInstanceData());
		}
	}
}

FGameCharacterInstanceData* UTRGameInstance::Server_GetCachedPlayerInstanceData(ATRPlayerController* TRPlayer)
{
	FGameCharacterInstanceData* Result = nullptr;
	if (TRPlayer->PlayerState)
	{
		FString PlayerId = TRPlayer->PlayerState->GetUniqueId().ToString();
		TR_PRINT_FSTRING("find %s", *PlayerId);
		Result = Server_CachedPlayersInstanceData.Find(PlayerId);
	}
	return Result;
}

void UTRGameInstance::Local_SetInvUIInfoText(FString Text)
{
	Local_InvUIInfoText = Text;
}

void UTRGameInstance::Server_OnDescendingDungeon(int32 NewDepth)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_OnDescendingDungeon - GameInstance has no world set! Aborting."));
		return;
	}
	if (!GetWorld()->GetAuthGameMode())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_OnDescendingDungeon - Client should never call this function!"));
		return;
	}
	if (NewDepth < 0)
	{
		// 음수는 층계를 변환하지 않음을 의미
		return;
	}
	Server_ChangeDungeonDepthTo(NewDepth);
}

void UTRGameInstance::Server_ChangeDungeonDepthTo(int32 TargetDepth)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_ChangeDungeonDepth - GameInstance has no world set! Aborting."));
		return;
	}
	if (!GetWorld()->GetAuthGameMode())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_ChangeDungeonDepth - Client should never call this function!"));
		return;
	}
	if (TargetDepth < Server_CurrDungeonDepth)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_ChangeDungeonDepth - Ascending the dungeon is not intended in current gameplay design! Please check."));
	}
	else if (TargetDepth == Server_CurrDungeonDepth)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_ChangeDungeonDepth - TargetDepth is same as Server_CurrDungeonDepth. This might be unintentional."));
	}
	Server_CurrDungeonDepth = TargetDepth;
}

void UTRGameInstance::Local_OnLevelTransitionBegin(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	if (bWaitingForLevelTransitionComplete) return;
	bWaitingForLevelTransitionComplete = true;
	UE_LOG(LogTemp, Warning, TEXT("Local_OnLevelTransitionBegin - Auth: %d"), !GetWorld()->IsNetMode(ENetMode::NM_Client));

	// 컨트롤 폰과 바인딩된 위젯 제거
	ATRPlayerController* LocalController = Cast<ATRPlayerController>(GetFirstLocalPlayerController());
	if (LocalController)
	{
		LocalController->Local_DerefPawnBoundedWidgets(LocalController->GetPawn());
	}

	// 드래그 드랍 액션 전부 취소; 이는 페이로드에 게임 로직 오브젝트가 남아있을 경우 잘못된 메모리를 참조하게 되기 때문임
	// 이는 크래시 혹은 메모리 누수로 이어질 수 있음
	Local_CancelAllDragDrops();

	// 로딩 스크린
	Local_ToggleLoadingScreenWidget(true);
}

void UTRGameInstance::Local_OnLevelTransitionComplete(UWorld* NewWorld)
{
	if (!NewWorld) return;
	if (!bWaitingForLevelTransitionComplete) return;
	bWaitingForLevelTransitionComplete = false;
	UE_LOG(LogTemp, Warning, TEXT("Local_OnLevelTransitionComplete - Auth: %d"), !NewWorld->IsNetMode(ENetMode::NM_Client));

	Local_ToggleLoadingScreenWidget(false);
}

void UTRGameInstance::Local_ToggleLoadingScreenWidget(bool bVisibility)
{
	if (bVisibility && LoadingScreenWidgetClass)
	{
		LoadingScreenWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenWidgetClass);
		if (LoadingScreenWidget)
		{
			LoadingScreenWidget->AddToViewport();
		}
		return;
	}

	// GC 처리가 늦어질 경우 잔여 레퍼런스 제거
	if (!bVisibility)
	{
		// NOTE: RemoveFromParent는 유효하지 않다; 
		// 기존 위젯 생성 시 이전 레벨(삭제된 월드)을 부모로 만들기 때문에 자연스럽게 GC에 의해 제거된다
		// 여기서 별도로 삭제 요청을 해서는 안된다
		LoadingScreenWidget = nullptr;
		return;
	}
}

const FString& UTRGameInstance::Local_GetInvUIInfoText()
{
	return Local_InvUIInfoText;
}

void UTRGameInstance::Local_CancelAllDragDrops()
{
	if (FSlateApplication::Get().IsDragDropping())
	{
		FSlateApplication::Get().CancelDragDrop();
	}
}
