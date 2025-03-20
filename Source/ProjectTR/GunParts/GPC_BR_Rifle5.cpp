// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Rifle5.h"
#include "TRMacros.h"

UGPC_BR_Rifle5::UGPC_BR_Rifle5()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_5));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 12.0f;
	DeltaDmgAllyDirect = 0.0f;
}