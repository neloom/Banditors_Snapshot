// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Shotgun8.h"
#include "TRMacros.h"

UGPC_BR_Shotgun8::UGPC_BR_Shotgun8()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SHOTGUN_8));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 66.0f;
	DeltaDmgAllyDirect = -20.0f;
}