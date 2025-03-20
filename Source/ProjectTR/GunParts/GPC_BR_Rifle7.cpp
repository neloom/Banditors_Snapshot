// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Rifle7.h"
#include "TRMacros.h"

UGPC_BR_Rifle7::UGPC_BR_Rifle7()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_7));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = -10.0f;
	DeltaDmgAllyDirect = -10.0f;
}