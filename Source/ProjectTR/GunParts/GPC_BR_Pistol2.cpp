// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Pistol2.h"
#include "TRMacros.h"

UGPC_BR_Pistol2::UGPC_BR_Pistol2()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_PISTOL_2));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 15.0f;
	DeltaDmgAllyDirect = 0.0f;
}