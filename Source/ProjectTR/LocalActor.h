// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LocalActor.generated.h"

/*
* 특정 호스트에만 존재하는, 어떠한 동기화도 처리되지 않는 액터
* 서버 정보를 사용해야 하는 경우 생성 전 미리 정보를 받아와 생성 후 그 값을 전달해야 한다
*/
UCLASS()
class PROJECTTR_API ALocalActor : public AActor
{
	GENERATED_BODY()
	
public:
	ALocalActor();
};
