// Copyright (C) 2024 by Haguk Kim


#include "LobbyFallVolume.h"
#include "FPSCharacter.h"
#include "GameFramework/GameModeBase.h"

void ALobbyFallVolume::ProcessPlayerOverlap(AFPSCharacter* Player, bool bAllPlayerOverlapped)
{
	if (!HasAuthority()) return;
	if (!Player || !Player->GetController()) return;

	FVector SpawnLocation;
	AActor* PlayerStartActor = GetWorld()->GetAuthGameMode()->FindPlayerStart(Player->GetController());
	Player->SetActorLocation(PlayerStartActor->GetActorLocation());
}
