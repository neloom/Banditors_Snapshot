// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Shotgun3.h"
#include "TRMacros.h"

UGPC_BR_Shotgun3::UGPC_BR_Shotgun3()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SHOTGUN_3));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 2.0f;
	DeltaDmgAllyDirect = -2.0f;
}