// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Rifle11.h"
#include "TRMacros.h"

UGPC_BR_Rifle11::UGPC_BR_Rifle11()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_11));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 33.0f;
	DeltaDmgAllyDirect = 12.0f;
}