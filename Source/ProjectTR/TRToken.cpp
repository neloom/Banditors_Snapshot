// Copyright (C) 2024 by Haguk Kim


#include "TRToken.h"

ATRToken::ATRToken()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent->SetGenerateOverlapEvents(true); // 제단
}

void ATRToken::Initialize(int32 TokenTier)
{
	Tier = TokenTier;
}

void ATRToken::BeginPlay()
{
	Super::BeginPlay();
	
}


