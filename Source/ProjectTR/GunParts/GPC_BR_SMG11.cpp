// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG11.h"
#include "TRMacros.h"

UGPC_BR_SMG11::UGPC_BR_SMG11()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_11));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = -9.0f;
	DeltaDmgAllyDirect = -9.0f;
}