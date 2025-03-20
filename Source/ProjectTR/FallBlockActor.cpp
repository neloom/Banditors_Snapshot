// Copyright (C) 2024 by Haguk Kim


#include "FallBlockActor.h"
#include "ProjectTRGameModeBase.h"
#include "TRPlayerController.h"

void AFallBlockActor::OnTriggered()
{
	if (!HasAuthority()) return;
	Destroy();
	ForceNetUpdate();
}
