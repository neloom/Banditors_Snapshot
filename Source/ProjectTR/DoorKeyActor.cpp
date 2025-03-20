// Copyright (C) 2025 by Haguk Kim


#include "DoorKeyActor.h"
#include "FPSCharacter.h"
#include "TRGameState.h"
#include "LockedDoorActor.h"
#include "EngineUtils.h"

void ADoorKeyActor::OnMuzzleTriggered(AGameCharacter* TriggeredBy)
{
	UWorld* World = GetWorld();
	AFPSCharacter* TriggerChar = Cast<AFPSCharacter>(TriggeredBy);
	if (World && TriggerChar)
	{
		ATRGameState* TRGS = World->GetGameState<ATRGameState>();
		if (TRGS)
		{
			TR_PRINT_FSTRING("Key obtained: %d", KeyId);

			TRGS->Server_AddDoorKey(KeyId);
			bCanBeDestroyed = true;
			Destroy();
		}
	}
}

void ADoorKeyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bCanBeDestroyed) return;

	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ALockedDoorActor> It(World); It; ++It)
		{
			ALockedDoorActor* LockedDoor = *It;
			if (LockedDoor->GetDoorId() == GetKeyId())
			{
				UE_LOG(LogTemp, Error, TEXT("ADoorKeyActor - Unexpected key destruction! Force-opening matching locked door(s) to prevent permanent locked door(s)!"));
				LockedDoor->Unlock();
			}
		}
	}
	return;
}
