// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "Math/Color.h"
#include "LocalDamageNumber.h"
#include "TRGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ATRGameState : public AGameStateBase // Mixing AGameState with AGameModeBase is not compatible
{
	GENERATED_BODY()

#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ATRGameState, DungeonTimeLeft);
		DOREPLIFETIME(ATRGameState, DungeonDoorKeys);
	}
#pragma endregion

/* Gameplay */
protected:
	// 현재 던전 입장 후 경과 시간 (초)
	UPROPERTY(ReplicatedUsing = OnRep_DungeonTimeLeft, BlueprintReadOnly)
	int32 DungeonTimeLeft;

public:
	// 현재 던전에서 획득한 보유 키들의 목록
	// 각 키들은 레벨 단위로만 관리된다
	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<int32> DungeonDoorKeys;

public:
	// 던전 시간 Getters, Setters
	int32 Local_GetDungeonTimeLeft() { return DungeonTimeLeft; }
	void Server_SetDungeonTimeLeft(int32 Value);

	// 스트링 형태로 변환해 반환
	UFUNCTION(BlueprintCallable)
	FString GetDungeonTimeString();

	// 레드모드 진입 시 각 호스트에서 처리해주어야 하는 로직을 처리한다
	void Server_ProcessRedModeEnter();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ProcessRedModeEnter();

	// 레벨 내의 모든 포인트 라이트들의 밝기 및 색상을 재설정한다
	// 색상 포인터가 nullptr일 경우 변경하지 않는다
	void Local_SetAllPointLightSettings(float AmbBrightMult, float NonAmbBrightMult, FLinearColor* AmbColor = nullptr, FLinearColor* NonAmbColor = nullptr);

	// 던전 키 획득 / 소비
	// 주어진 Id의 키를 추가한다
	void Server_AddDoorKey(int32 KeyId);

	// 주어진 Id의 키를 소비하고, 소비 여부를 반환한다
	bool Server_UseDoorKey(int32 KeyId);

protected:
	UFUNCTION()
	void OnRep_DungeonTimeLeft();
	void Local_OnDungeonTimeLeftUpdated();

/* UI */
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDamageNumberWidget> DefaultDamageNumberWidgetClass = nullptr;

protected:
	// 사용 가능한 풀
	// TODO: WidgetClass에 따라 다른 풀을 사용하면 조금 더 효율적으로 관리할 수 있음 (현재는 풀링된 것과 위젯이 다를 경우 새로 생성함)
	TQueue<ALocalDamageNumber*> UsableLocalDamageNumberPool;

public:
	// NOTE: 풀
	ALocalDamageNumber* Local_DisplayDamageNumber(UWorld* World, TSubclassOf<class UDamageNumberWidget> WidgetClass, const FTransform& Transform, int32 DmgValue, bool bForceNewInstance);
};
