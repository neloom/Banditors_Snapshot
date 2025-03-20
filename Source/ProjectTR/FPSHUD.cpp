// Copyright (C) 2024 by Haguk Kim


#include "FPSHUD.h"
#include "TRCrosshair.h"

void AFPSHUD::BeginPlay()
{
	Super::BeginPlay();

	if (CrosshairClass)
	{
		Crosshair = CreateWidget<UTRCrosshair>(GetWorld(), CrosshairClass);
	}
	if (Crosshair)
	{
		Crosshair->AddToViewport();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AFPSHUD::DrawHUD - Something went wrong, crosshair generation failed!"));
	}
}

void AFPSHUD::DrawHUD()
{
	Super::DrawHUD();
}
