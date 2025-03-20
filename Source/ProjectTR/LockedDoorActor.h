// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TriggerOpenDoor.h"
#include "LockedDoorActor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ALockedDoorActor : public ATriggerOpenDoor
{
	GENERATED_BODY()

public:
	virtual void OnMuzzleTriggered(class AGameCharacter* TriggeredBy) override;
	void Unlock();

	void SetDoorId(int32 Id) { if (DoorId >= 0) { UE_LOG(LogTemp, Error, TEXT("SetDoorId - DoorId has already been set, overriding! Something went wrong!")); } DoorId = Id; }
	int32 GetDoorId() { return DoorId; }

	bool IsLocked() { return bIsLocked; }

protected:
	// 문에 할당된 아이디; 0 이상의 정수값만이 유효한 아이디로 취급된다
	UPROPERTY(BlueprintReadOnly)
	int32 DoorId = -1;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLocked = true;
};
