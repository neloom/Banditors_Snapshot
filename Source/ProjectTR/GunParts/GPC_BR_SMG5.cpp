// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG5.h"
#include "TRMacros.h"

UGPC_BR_SMG5::UGPC_BR_SMG5()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_5));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 7.0f;
	DeltaDmgAllyDirect = -7.0f;
}