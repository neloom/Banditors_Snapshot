// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Rifle12.h"
#include "TRMacros.h"

UGPC_BR_Rifle12::UGPC_BR_Rifle12()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_12));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 24.0f;
	DeltaDmgAllyDirect = 6.0f;
}