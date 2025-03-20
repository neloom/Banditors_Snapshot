// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "MuzzleTriggeredActor.h"
#include "DoorKeyActor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ADoorKeyActor : public AMuzzleTriggeredActor
{
	GENERATED_BODY()
	
public:
	virtual void OnMuzzleTriggered(class AGameCharacter* TriggeredBy) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetKeyId(int32 Id) { if (KeyId >= 0) { UE_LOG(LogTemp, Error, TEXT("SetKeyId - KeyId has already been set, overriding! Something went wrong!")); } KeyId = Id; }
	int32 GetKeyId() { return KeyId; }
	void SetCanBeDestroyed(bool Value) { bCanBeDestroyed = Value; }

protected:
	// 열쇠에 할당된 아이디; 0 이상의 정수값만이 유효한 아이디로 취급된다
	UPROPERTY(BlueprintReadOnly)
	int32 KeyId = -1;

private:
	bool bCanBeDestroyed = false;
};
