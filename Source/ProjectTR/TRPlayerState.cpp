// Copyright (C) 2024 by Haguk Kim

#include "TRPlayerState.h"
#include "ProjectTRGameModeBase.h"

void ATRPlayerState::OnRep_PlayerOutUpdate()
{
	// TODO: 플레이어 아웃 시 공통 로직 작성
}

void ATRPlayerState::SetIsOut(bool Value)
{
	if (!HasAuthority()) return;
	
	bIsOut = Value;
	AProjectTRGameModeBase* GameMode = Cast<AProjectTRGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->UpdateGameOverStatus();
	}
}
