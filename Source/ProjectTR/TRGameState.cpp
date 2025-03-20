// Copyright (C) 2024 by Haguk Kim


#include "TRGameState.h"
#include "TRUtils.h"
#include "TRMacros.h"
#include "Engine/PointLight.h"
#include "DamageNumberWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TRPlayerController.h"
#include "GameCharacter.h"
#include "TRHUDWidget.h"
#include "EngineUtils.h"

void ATRGameState::Server_SetDungeonTimeLeft(int32 Value)
{
	if (!HasAuthority()) return;
	if (Value == DungeonTimeLeft) return;
	DungeonTimeLeft = Value;
	
	// 서버의 경우 수동 호출
	Local_OnDungeonTimeLeftUpdated();
}

FString ATRGameState::GetDungeonTimeString()
{
    return TRUtils::TimeSecondsToString(DungeonTimeLeft);
}

void ATRGameState::Server_ProcessRedModeEnter()
{
	if (!HasAuthority()) return;
	Multicast_ProcessRedModeEnter();
}

void ATRGameState::Multicast_ProcessRedModeEnter_Implementation()
{
	// 레드모드 진입 시 라이트 효과 부여
	FLinearColor Red = FLinearColor(255, 0, 0);
	Local_SetAllPointLightSettings(1.0f, 2.0f, &Red, nullptr);
}

void ATRGameState::Local_SetAllPointLightSettings(float AmbBrightMult, float NonAmbBrightMult, FLinearColor* AmbColor, FLinearColor* NonAmbColor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	for (TActorIterator<APointLight> It(World); It; ++It)
	{
		APointLight* PointLight = *It;
		if (PointLight)
		{
			// 액터 인스턴스 태그에 "Ambient"를 추가해주어야 한다
			if (!PointLight->ActorHasTag("Ambient")) // ambient lights
			{
				PointLight->SetBrightness(PointLight->GetBrightness() * AmbBrightMult);
				if (AmbColor) PointLight->SetLightColor(*AmbColor);
			}
			else
			{
				PointLight->SetBrightness(PointLight->GetBrightness() * NonAmbBrightMult);
				if (NonAmbColor) PointLight->SetLightColor(*NonAmbColor);
			}
		}
	}
}

void ATRGameState::Server_AddDoorKey(int32 KeyId)
{
	if (DungeonDoorKeys.Contains(KeyId))
	{
		UE_LOG(LogTemp, Error, TEXT("Server_AddDoorKey - Key id %d already exists! Aborting."), KeyId);
		return;
	}
	DungeonDoorKeys.Add(KeyId);
}

bool ATRGameState::Server_UseDoorKey(int32 KeyId)
{
	return DungeonDoorKeys.Remove(KeyId) > 0;
}

void ATRGameState::OnRep_DungeonTimeLeft()
{
	Local_OnDungeonTimeLeftUpdated();
}

void ATRGameState::Local_OnDungeonTimeLeftUpdated()
{
	UWorld* World = GetWorld();
	if (!World) return;
	// UI 업데이트 (서버,클라)
	ATRPlayerController* TRPC = World->GetFirstLocalPlayerFromController<ATRPlayerController>();
	if (TRPC)
	{
		AGameCharacter* GameChar = Cast<AGameCharacter>(TRPC->GetCharacter());
		if (GameChar && GameChar->Local_GetBoundHUDWidget().IsValid())
		{
			GameChar->Local_GetBoundHUDWidget()->UpdateDungeonTimer();
		}
	}
}

ALocalDamageNumber* ATRGameState::Local_DisplayDamageNumber(UWorld* World, TSubclassOf<UDamageNumberWidget> WidgetClass, const FTransform& Transform, int32 DmgValue, bool bForceNewInstance)
{
	if (!World || !WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Local_DisplayDamageNumber - Invalid arguments!"));
		return nullptr;
	}

	ALocalDamageNumber* ReturnInst = nullptr;
	if (bForceNewInstance || UsableLocalDamageNumberPool.IsEmpty())
	{
		FActorSpawnParameters SpawnParam;
		SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ReturnInst = World->SpawnActor<ALocalDamageNumber>(
			ALocalDamageNumber::StaticClass(),
			Transform,
			SpawnParam
		);

		if (!ReturnInst)
		{
			UE_LOG(LogTemp, Error, TEXT("Local_DisplayDamageNumber - LocalDamageNumber actor spawn failed!"));
			return nullptr;
		}

		TR_PRINT("Local_DisplayDamageNumber -  New instance");
	}
	else
	{
		ALocalDamageNumber* DmgNumberActor = *UsableLocalDamageNumberPool.Peek();
		UsableLocalDamageNumberPool.Pop();
		if (!IsValid(DmgNumberActor))
		{
			UE_LOG(LogTemp, Error, TEXT("Local_DisplayDamageNumber - LocalDamageNumber actor that was in the usable pool is invalid!"));
			ReturnInst = Local_DisplayDamageNumber(World, WidgetClass, Transform, DmgValue, bForceNewInstance);
		}
		else
		{
			ReturnInst = DmgNumberActor;
		}

		ReturnInst->SetActorTransform(Transform);
		TR_PRINT("Local_DisplayDamageNumber -  Popped from pool");
	}

	ReturnInst->SetWidgetClassAs(WidgetClass);
	ReturnInst->ReturnPool = &UsableLocalDamageNumberPool;
	ReturnInst->StartDisplay(DmgValue);
	return ReturnInst;
}
