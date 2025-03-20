// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Rifle2.h"
#include "TRMacros.h"

UGPC_BR_Rifle2::UGPC_BR_Rifle2()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_2));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 35.0f;
	DeltaDmgAllyDirect = 20.0f;
}