// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Shotgun4.h"
#include "TRMacros.h"

UGPC_BR_Shotgun4::UGPC_BR_Shotgun4()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SHOTGUN_4));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 40.0f;
	DeltaDmgAllyDirect = -10.0f;
}