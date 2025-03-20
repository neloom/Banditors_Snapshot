// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Shotgun1.h"
#include "TRMacros.h"

UGPC_BR_Shotgun1::UGPC_BR_Shotgun1()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SHOTGUN_1));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 20.0f;
	DeltaDmgAllyDirect = 10.0f;
}