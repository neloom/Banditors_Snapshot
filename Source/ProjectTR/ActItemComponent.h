// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "ActItemComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTTR_API UActItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActItemComponent();

#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UActItemComponent, TriggerInterval);
	}
#pragma endregion

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 현재 상태에서 트리거가 허용되는지 확인한다
	virtual bool Host_CanTrigger(class AGameCharacter* Invoker);

	// 실행 로직
	// 로직의 성공 여부를 반환한다
	virtual bool Trigger(class AGameCharacter* Invoker);

	// 플레이어 로직
	// 로직의 성공 여부를 반환한다
	virtual bool TriggeredByPlayer(class AFPSCharacter* PlayerPawn);

	// AI 로직
	// 로직의 성공 여부를 반환한다
	virtual bool TriggeredByAI(class AGameCharacter* AIPawn);

	// 중단 로직
	// 로직의 성공 여부를 반환한다
	virtual bool Stop(class AGameCharacter* Invoker);

	// 중단 로직
	// 로직의 성공 여부를 반환한다
	virtual bool StoppedByPlayer(class AFPSCharacter* PlayerPawn);

	// 중단 로직
	// 로직의 성공 여부를 반환한다
	virtual bool StoppedByAI(class AGameCharacter* AIPawn);

	// FX 로직
	// 아이템 사용 시작 시의 FX 로직을 처리한다
	virtual void Local_PlayFx(class AGameCharacter* Invoker);

	// FX 로직
	// 아이템 사용 중단 시의 FX 로직을 처리한다
	virtual void Local_StopFx(class AGameCharacter* Invoker);

	// 클라이언트에서 호출된다
	// 서버로부터 레플리케이션 되지 않지만 미리 계산해야 하는 값들을 처리한다
	virtual void Local_OnClientSimulation(class AGameCharacter* FireActor);

	// 이 컴포넌트가 부모 아이템의 Primary 컴포넌트인지 반환한다
	bool IsComponentPrimary() const;

	// 주어진 트리거 대기 시간 이후 델리게이트를 호출하도록 타이머를 가동한다
	void Server_StartTriggerTimer();

protected:
	// 트리거 시간이 경과했을 때 호출할 델리게이트
	void Server_OnTriggerTimerPassed();

	// 트리거 상태를 변경한다
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateTriggerIntervalState(bool bValue);

public:
	// Trigger interval의 client prediction
	// 값이 true일 경우 Host단 CanTrigger 체크가 fail한다
	// 서버의 경우 이 값을 수정하지 않는다
	bool bClient_AssumeTriggerBlocked = false;

	// 입력에 대해 트리거 처리를 한 이후 다음 입력 트리거까지 대기해야 하는 시간
	// 마우스를 빠르게 클릭해 대기 없이 트리거하는 행위를 막는 장치이다
	UPROPERTY(Replicated, BlueprintReadOnly)
	float TriggerInterval = 0.5f;

protected:
	// 트리거 주기 관리용 타이머
	// 타이머는 서버에서만 동작한다
	FTimerHandle Server_TriggerIntervalTimer;

	// 트리거 주기 델리게이트
	FTimerDelegate Server_TriggerIntervalDelegate;

private:
	// 현재 트리거 시간이 경과해 다음 트리거를 처리할 수 있는 상태인지 여부
	// true면 격발 가능하다
	// 이 값은 Server-authoritative하므로 클라에서 Write 연산을 수행해서는 안된다
	// 클라의 TriggerInterval prediction은 bClient_AssumeTriggerBlocked를 통해 처리한다
	// Authority는 UPROPERTY 대신 RPC를 사용한다; 안정성 및 반응성을 보장하기 위함
	bool bHost_HasTriggerIntervalPassed = true;
};
