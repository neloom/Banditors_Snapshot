// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "RoomCustomData.h"
#include "TRRoomCustomData.generated.h"


/* 던전 룸 타입을 나타내는 Enum */
UENUM(BlueprintType)
enum class ETRRoomType : uint8
{
	RT_Default UMETA(DisplayName = "Default room"), // 일반적인 방에 해당
	RT_Lobby UMETA(DisplayName = "Lobby room"), // 넓은 탁 트인 구조의 방에 해당
	RT_Passage UMETA(DisplayName = "Passage room"), // 복도와 같은 경로 역할의 룸
	RT_Misc UMETA(DisplayName = "Misc room") // 그 외
};

/**
 * 방에 대한 추가적인 정보를 표현하기 위해 사용됨
 * DamageType 같이 static하게 사용되며, 각 타입은 ETRRoomType Enum과 매칭되어야 한다
 */
UCLASS()
class PROJECTTR_API UTRRoomCustomData : public URoomCustomData
{
	GENERATED_BODY()
};

UCLASS()
class PROJECTTR_API UTRRoomType_Default : public UTRRoomCustomData
{
	GENERATED_BODY()
};

UCLASS()
class PROJECTTR_API UTRRoomType_Lobby : public UTRRoomCustomData
{
	GENERATED_BODY()
};

UCLASS()
class PROJECTTR_API UTRRoomType_Passage : public UTRRoomCustomData
{
	GENERATED_BODY()
};

UCLASS()
class PROJECTTR_API UTRRoomType_Misc : public UTRRoomCustomData
{
	GENERATED_BODY()
};