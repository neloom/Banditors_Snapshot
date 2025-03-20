// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Shotgun5.h"
#include "TRMacros.h"

UGPC_BR_Shotgun5::UGPC_BR_Shotgun5()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SHOTGUN_5));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 50.0f;
	DeltaDmgAllyDirect = 50.0f;
}