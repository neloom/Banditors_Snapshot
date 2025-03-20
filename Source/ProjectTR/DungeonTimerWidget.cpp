// Copyright (C) 2024 by Haguk Kim


#include "DungeonTimerWidget.h"
#include "TRGameState.h"

void UDungeonTimerWidget::Update()
{
	if (!GetWorld()) return;
	if (!TimerText) return;
	ATRGameState* TRGS = GetWorld()->GetGameState<ATRGameState>();
	if (!TRGS)
	{
		return;
	}
	TimerText->SetText(FText::FromString(TRGS->GetDungeonTimeString()));
}
