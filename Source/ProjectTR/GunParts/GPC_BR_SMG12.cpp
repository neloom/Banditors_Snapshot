// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG12.h"
#include "TRMacros.h"

UGPC_BR_SMG12::UGPC_BR_SMG12()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_12));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 47.0f;
	DeltaDmgAllyDirect = 0.0f;
}