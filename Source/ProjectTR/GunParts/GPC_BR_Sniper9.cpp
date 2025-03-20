// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Sniper9.h"
#include "TRMacros.h"

UGPC_BR_Sniper9::UGPC_BR_Sniper9()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SNIPER_9));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 47.0f;
	DeltaDmgAllyDirect = 47.0f;
}