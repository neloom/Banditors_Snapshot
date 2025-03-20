// Copyright (C) 2024 by Haguk Kim


#include "DescendVolume.h"
#include "FPSCharacter.h"
#include "ProjectTRGameModeBase.h"
#include "TRMacros.h"
#include "TRPlayerController.h"
#include "GameFramework/GameModeBase.h"

void ADescendVolume::ProcessPlayerOverlap(AFPSCharacter* Player, bool bAllPlayersOverlapped)
{
	if (!HasAuthority()) return;
	if (!Player || !Player->GetController()) return;

	// 아직 전부 오버랩되지 않았을 경우 개인 알림
	if (!bAllPlayersOverlapped)
	{
		ATRPlayerController* TRController = Cast<ATRPlayerController>(Player->GetController());
		if (TRController)
		{
			FString AlertText = FString::Printf(TEXT("You can't activate the portal alone. All active players must be present."));
			TRController->Local_AlertTextRPC(AlertText, 5.0f);
		}
	}
}

void ADescendVolume::ProcessAllPlayersOverlap()
{
	if (!HasAuthority()) return;
	AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(GetWorld()->GetAuthGameMode());
	if (TRGM)
	{
		int32 TargetDepth = TRGM->GetCurrentDungeonDepth() + 1;
		TRGM->ChangeGameLevel(TRGM->GetLevelNameOfDepth(TargetDepth), TargetDepth);
	}
}
