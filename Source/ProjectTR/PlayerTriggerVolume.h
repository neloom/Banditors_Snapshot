// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "DungeonActor.h"
#include "PlayerTriggerVolume.generated.h"

/**
 * NOTE: 이 클래스를 블루프린트에서 재상속하는 것은 가급적 지양해야 한다
 * 볼륨은 팩토리에 의해 생성되기 때문이다
 */
UCLASS()
class PROJECTTR_API APlayerTriggerVolume : public ATriggerVolume
{
	GENERATED_BODY()
	
public:
	APlayerTriggerVolume();

	// 오버랩 시 처리 로직
	UFUNCTION()
	void Server_OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	// 오버랩 해제 시 처리 로직
	UFUNCTION()
	void Server_OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	// 현재 이 볼륨에 모든 플레이어들이 오버랩 된 상태인지 여부
	UFUNCTION()
	bool IsAllPlayerOverlapped();

	/* Setters */
	UFUNCTION()
	void SetPairActor(class ADungeonActor* Actor) { Pair = Actor; }

protected:
	// 플레이어 오버랩 로직
	void Server_OnPlayerOverlapBegin(class AFPSCharacter* Player);
	void Server_OnPlayerOverlapEnd(class AFPSCharacter* Player);

	// 플레이어 오버랩 커스텀 로직
	virtual void ProcessPlayerOverlap(class AFPSCharacter* Player, bool bAllPlayersOverlapped);
	virtual void ProcessAllPlayersOverlap();

protected:
	// 현재 오버랩된 플레이어 수
	int OverlappedPlayerCount = 0;

	// 이 볼륨이 영향을 미칠 대상 액터
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TWeakObjectPtr<ADungeonActor> Pair;

	// Pair를 언제 트리거 할지 조정
	// 플레이어 오버랩 발생 시 트리거
	UPROPERTY(EditInstanceOnly)
	bool bTriggerPairWhenPlayerOverlap = false;

	// 모든 플레이어가 오버랩 할 경우 트리거
	// bTriggerPairWhenPlayerOverlap가 true인 경우 무시됨
	UPROPERTY(EditInstanceOnly)
	bool bTriggerPairWhenAllPlayerOverlap = false;

	// 최초 1회만 트리거 할 지 여부
	UPROPERTY(EditInstanceOnly)
	bool bActivateOnceOnly = false;
	bool bHasActivatedOnceOrMore = false;
};
