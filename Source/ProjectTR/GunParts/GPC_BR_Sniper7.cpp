// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Sniper7.h"
#include "TRMacros.h"

UGPC_BR_Sniper7::UGPC_BR_Sniper7()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SNIPER_7));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 26.0f;
	DeltaDmgAllyDirect = 9.0f;
}