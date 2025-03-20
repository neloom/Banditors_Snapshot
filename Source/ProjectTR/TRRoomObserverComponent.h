// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/RoomObserverComponent.h"
#include "TRRoomObserverComponent.generated.h"

/**
 * 모든 감지 이벤트는 서버에서만 처리한다
 */
UCLASS()
class PROJECTTR_API UTRRoomObserverComponent : public URoomObserverComponent
{
	GENERATED_BODY()

	UTRRoomObserverComponent();

public:
	// 컴포넌트를 가동한다
	// 델리게이트를 바인딩하고 동작을 개시한다
	virtual void Activate(bool bReset = false) override;

protected:
	// 해당 룸 레벨이 유효한지 검증해 결과를 반환한다
	bool IsValidRoomLevel(ARoomLevel* RoomLevel);

	// 이 컴포넌트를 부착한 액터가 룸에 진입했을 때의 로직을 처리한다
	UFUNCTION()
	void Server_ProcessActorEnterRoom(ARoomLevel* RoomLevel, AActor* Actor);

	// 이 컴포넌트를 부착한 액터가 룸에서 탈출했을 때의 로직을 처리한다
	UFUNCTION()
	void Server_ProcessActorExitRoom(ARoomLevel* RoomLevel, AActor* Actor);

	// 게임 캐릭터 감지 시의 로직을 작성한다
	void Server_ProcessGameCharacterEnterRoom(ARoomLevel* RoomLevel, class AGameCharacter* Character);
	void Server_ProcessGameCharacterExitRoom(ARoomLevel* RoomLevel, class AGameCharacter* Character);
};
