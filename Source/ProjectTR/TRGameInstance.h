// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TRStructs.h"
#include "TRGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UTRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UTRGameInstance();

	UFUNCTION(BlueprintCallable)
	class USoundSubsystem* GetSoundManager();

public:
	// 기존 캐시된 값들을 리셋하고 주어진 플레어어들의 인스턴스 정보들을 캐싱한다
	void Server_CachePlayersInstanceData(const TArray<class ATRPlayerController*>& TRPlayers);

	// 캐시된 정보로부터 플레이어 인스턴스 데이터를 찾아 반환한다
	FGameCharacterInstanceData* Server_GetCachedPlayerInstanceData(class ATRPlayerController* TRPlayer);

	// 층계 변경 시도시 호출
	void Server_OnDescendingDungeon(int32 NewDepth);
	void Server_ChangeDungeonDepthTo(int32 TargetDepth);
	int32 Server_GetDungeonDepth() { return Server_CurrDungeonDepth; }

	// UI Infobox에 표기될 텍스트를 갱신한다
	UFUNCTION(BlueprintCallable)
	void Local_SetInvUIInfoText(FString Text);

	// UI Infobox에 표기해야 할 텍스트를 읽어온다
	UFUNCTION(BlueprintCallable)
	const FString& Local_GetInvUIInfoText();

	// 현재 처리중인 모든 드래그 드랍 오퍼레이션을 취소한다
	void Local_CancelAllDragDrops();

protected:
	// 로컬 레벨 트랜지션 로직
	// 서버 및 클라이언트 모두 호출됨
	// NOTE: 이 함수들은 상황에 따라 단일 레벨 트랜지션임에도 여러번 호출될 수 있다는 점 유의
	UFUNCTION()
	void Local_OnLevelTransitionBegin(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel);

	UFUNCTION()
	void Local_OnLevelTransitionComplete(UWorld* NewWorld);

	// 로딩 스크린 위젯 관리
	// 인자가 true일 경우 새 위젯을 생성한다
	// 이때 호출 시점은 레벨 트랜지션의 시작 이전이어야 한다
	// 그래야 기존 월드를 parent로 위젯을 제작해 자연스럽게 트랜지션 이후 GC에 의해 위젯이 제거되게 만들 수 있다
	void Local_ToggleLoadingScreenWidget(bool bVisibility);

private:
	// Local_OnLevelTransitionBegin, Local_OnLevelTransitionComplete 호출의 1대1 맵핑을 보장하기 위한 플래그
	bool bWaitingForLevelTransitionComplete = false;

protected:
	// 플레이어별 데이터 캐싱
	TMap<FString, FGameCharacterInstanceData> Server_CachedPlayersInstanceData;

	// 던전 깊이 (2는 지하 2층을 의미)
	// NOTE: 이 값은 최초 던전을 생성할 때에만 사용되며, 
	// 게임플레이 상에서 던전 깊이를 구하기 위해선 게임모드의 변수를 사용할 것
	int32 Server_CurrDungeonDepth = 0;

	// UI Infobox에 표기할 텍스트 내용
	FString Local_InvUIInfoText;

	// 로딩 스크린 위젯
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	TSubclassOf<class UUserWidget> LoadingScreenWidgetClass;
	TObjectPtr<class UUserWidget> LoadingScreenWidget = nullptr;
};
