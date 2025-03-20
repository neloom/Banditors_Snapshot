// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Pistol1.h"
#include "TRMacros.h"

UGPC_BR_Pistol1::UGPC_BR_Pistol1()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_PISTOL_1));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 5.0f;
	DeltaDmgAllyDirect = 0.0f;
}