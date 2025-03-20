// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG9.h"
#include "TRMacros.h"

UGPC_BR_SMG9::UGPC_BR_SMG9()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_9));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 20.0f;
	DeltaDmgAllyDirect = -20.0f;
}