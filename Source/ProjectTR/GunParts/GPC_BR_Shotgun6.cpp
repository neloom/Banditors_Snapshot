// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Shotgun6.h"
#include "TRMacros.h"

UGPC_BR_Shotgun6::UGPC_BR_Shotgun6()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SHOTGUN_6));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 10.0f;
	DeltaDmgAllyDirect = 10.0f;
}