// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Shotgun7.h"
#include "TRMacros.h"

UGPC_BR_Shotgun7::UGPC_BR_Shotgun7()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SHOTGUN_7));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 0.0f;
	DeltaDmgAllyDirect = 0.0f;
}