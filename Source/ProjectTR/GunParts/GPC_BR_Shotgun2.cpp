// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Shotgun2.h"
#include "TRMacros.h"

UGPC_BR_Shotgun2::UGPC_BR_Shotgun2()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SHOTGUN_2));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 14.0f;
	DeltaDmgAllyDirect = 0.0f;
}