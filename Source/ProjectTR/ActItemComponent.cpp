// Copyright (C) 2024 by Haguk Kim


#include "ActItemComponent.h"
#include "GameCharacter.h"
#include "WieldItem.h"
#include "FPSCharacter.h"

// Sets default values for this component's properties
UActItemComponent::UActItemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 클라에서는 바인딩하더라도 애초에 Trigger()가 실행되지 않으며, 실행되더라도 로직이 처리되지 않는다
	Server_TriggerIntervalDelegate = FTimerDelegate::CreateUObject(this, &UActItemComponent::Server_OnTriggerTimerPassed);
}


// Called when the game starts
void UActItemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UActItemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UActItemComponent::Host_CanTrigger(AGameCharacter* Invoker)
{
	if (!IsValid(Invoker)) return false;
	if (!bHost_HasTriggerIntervalPassed) return false;

	// 클라이언트의 격발 시뮬레이션(FX)이 종료된 직후 아직 서버로부터 재격발 허가가 떨어지지 않은 상황인 경우
	if (!Invoker->IsLocallyControlled() && bClient_AssumeTriggerBlocked) return false;

	return true; 
	// 추가 로직 필요 시 오버라이드
}

bool UActItemComponent::Trigger(AGameCharacter* Invoker)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("UActItemComponent::Trigger - component owner is invalid, or the host is not authoritative."));
		return false;
	}
	if (!Host_CanTrigger(Invoker))
	{
		UE_LOG(LogTemp, Warning, TEXT("UActItemComponent::Trigger - Invalid request. This could happen if client is not fully in-sync with the server."));
		return false;
	}

	AFPSCharacter* PlayerPawn = Cast<AFPSCharacter>(Invoker);
	bool bTriggerResult = false;
	if (PlayerPawn) 
	{
		bTriggerResult = TriggeredByPlayer(PlayerPawn);
	}
	else 
	{
		bTriggerResult = TriggeredByAI(Invoker);
	}

	if (bTriggerResult)
	{
		// 트리거 타이머 설정
		Server_StartTriggerTimer();

		AWieldItem* WieldOwner = Cast<AWieldItem>(GetOwner());
		if (WieldOwner)
		{
			return WieldOwner->OnItemTriggerProcessed(this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UActItemComponent::Trigger - Component owner is not a child of AWieldItem! Please check."));
			return false;
		}
	}
	return true;
}

bool UActItemComponent::TriggeredByPlayer(AFPSCharacter* PlayerPawn)
{
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("TriggeredByPlayer - PlayerPawn is null!"));
		return false;
	}
	return true;
}

bool UActItemComponent::TriggeredByAI(AGameCharacter* AIPawn)
{
	if (!AIPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("TriggeredByAI - Pawn is null!"));
		return false;
	}
	return true;
}

bool UActItemComponent::Stop(AGameCharacter* Invoker)
{
	AFPSCharacter* PlayerPawn = Cast<AFPSCharacter>(Invoker);
	if (PlayerPawn) {
		return StoppedByPlayer(PlayerPawn);
	}
	else {
		return StoppedByAI(Invoker);
	}
}

bool UActItemComponent::StoppedByPlayer(AFPSCharacter* PlayerPawn)
{
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("TriggeredByPlayer - PlayerPawn is null!"));
		return false;
	}
	return true;
}

bool UActItemComponent::StoppedByAI(AGameCharacter* AIPawn)
{
	if (!AIPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("TriggeredByAI - Pawn is null!"));
		return false;
	}
	return true;
}

void UActItemComponent::Local_PlayFx(AGameCharacter* Invoker)
{
	// 필요 시 오버라이드
	return;
}

void UActItemComponent::Local_StopFx(AGameCharacter* Invoker)
{
	// 필요 시 오버라이드
	return;
}

void UActItemComponent::Local_OnClientSimulation(AGameCharacter* FireActor)
{
	bClient_AssumeTriggerBlocked = true;
}

bool UActItemComponent::IsComponentPrimary() const
{
	return (Cast<AWieldItem>(GetOwner())->PrimaryActComponent == this);
}

void UActItemComponent::Server_StartTriggerTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_StartTriggerTimer - World is missing!"));
		return;
	}
	if (World->GetNetMode() == ENetMode::NM_Client || !bHost_HasTriggerIntervalPassed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_StartTriggerTimer - Unintended behaviour!"));
	}
	Multicast_UpdateTriggerIntervalState(false);
	World->GetTimerManager().SetTimer(Server_TriggerIntervalTimer, Server_TriggerIntervalDelegate, TriggerInterval, true);
}

void UActItemComponent::Server_OnTriggerTimerPassed()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_OnTriggerTimerPassed - World is missing!"));
		return;
	}

	// 타이머 종료
	World->GetTimerManager().ClearTimer(Server_TriggerIntervalTimer);
	Multicast_UpdateTriggerIntervalState(true);
}

void UActItemComponent::Multicast_UpdateTriggerIntervalState_Implementation(bool bValue)
{
	bHost_HasTriggerIntervalPassed = bValue;

	// 클라이언트의 경우 Prediction 값을 조정한다
	AActor* CompOwner = GetOwner();
	if (CompOwner && !CompOwner->HasAuthority())
	{
		if (bHost_HasTriggerIntervalPassed)
		{
			bClient_AssumeTriggerBlocked = false;
		}
	}
}

