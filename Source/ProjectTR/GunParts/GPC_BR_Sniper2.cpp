// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Sniper2.h"
#include "TRMacros.h"

UGPC_BR_Sniper2::UGPC_BR_Sniper2()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SNIPER_2));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 43.0f;
	DeltaDmgAllyDirect = 0.0f;
}