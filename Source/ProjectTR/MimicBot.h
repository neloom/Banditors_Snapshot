// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BotCharacter.h"
#include "MimicBot.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API AMimicBot : public ABotCharacter
{
	GENERATED_BODY()
	
public:
	AMimicBot(const FObjectInitializer& ObjectInitializer);

	// 상자를 기반으로 생성되는 경우 상자의 데이터 중 필요한 값들을 가져와 초기화한다
	void SetupChestInfo(const TArray<FDropItem>& ChestRewards);
};
