// Copyright (C) 2025 by Haguk Kim


#include "LockedDoorActor.h"
#include "FPSCharacter.h"
#include "TRGameState.h"

void ALockedDoorActor::OnMuzzleTriggered(AGameCharacter* TriggeredBy)
{
	// 부모 로직 완전히 재정의
	UWorld* World = GetWorld();
	AFPSCharacter* TriggerChar = Cast<AFPSCharacter>(TriggeredBy);
	if (World && TriggerChar)
	{
		ATRGameState* TRGS = World->GetGameState<ATRGameState>();
		if (TRGS)
		{
			if (TRGS->Server_UseDoorKey(DoorId))
			{
				Unlock();
				TriggerThis(); // 문 열기
			}
		}
	}
}

void ALockedDoorActor::Unlock()
{
	if (!bIsLocked) return;

	TR_PRINT_FSTRING("Door unlocked: %d", DoorId);
	bIsLocked = false;
}
