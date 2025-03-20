// Copyright (C) 2024 by Haguk Kim


#include "TRSoul.h"
#include "FPSCharacter.h"
#include "TRPlayerController.h"
#include "SoulItemData.h"

ATRSoul::ATRSoul()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent->SetGenerateOverlapEvents(true); // 제단
}

void ATRSoul::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ATRSoul::RestoreFromItemData(UItemData* Data)
{
	if (!Super::RestoreFromItemData(Data)) return false;
	USoulItemData* SoulData = Cast<USoulItemData>(Data);
	if (!IsValid(SoulData))
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to restore a soul item from a non-SoulItemData %s."), *(Data->GetName()));
		return false;
	}
	if (!HasAuthority())
	{
		// 클라이언트는 아무 것도 복구하지 않음
		// TODO: 만약 클라에 복구해야 할 값이 생길 경우 현재 코드 블록에 진입하기 이전에 처리를 완료하게 작성하면 됨
		UE_LOG(LogTemp, Warning, TEXT("Client does not have anything to restore from soul item data. This is a normal behaviour."), *(Data->GetName()));
		return true;
	}
	if (!IsValid(SoulData->GetCachedCharacterClass()) || !IsValid(SoulData->GetCachedController()))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has invalid cached data! Aborting."), *(Data->GetName()));
		return false;
	}

	// 복구 로직
	if (SoulData->GetCachedCharacterClass())
	{
		Server_SetCharacterClass(SoulData->GetCachedCharacterClass());
	}
	if (SoulData->GetCachedController())
	{
		Server_SetController(SoulData->GetCachedController());
	}
	Server_SetInstanceData(SoulData->GetCachedInstanceData());
	return true;
}

bool ATRSoul::IsReadyToRespawnPlayer() const
{
	if (!IsValid(Server_CharacterClass) || !IsValid(Server_Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("IsReadyToRespawnPlayer - Lacking necessary data! %d %d"), (Server_CharacterClass != nullptr), (Server_Controller != nullptr));
		return false;
	}

	AFPSCharacter* FPSPawn = Cast<AFPSCharacter>(Server_Controller->GetPawn());
	if (FPSPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("IsReadyToRespawnPlayer - Controller already owns a FPSCharacter!"));
		return false;
	}

	return true;
}


