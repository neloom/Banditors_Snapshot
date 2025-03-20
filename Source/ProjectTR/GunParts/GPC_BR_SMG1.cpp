// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG1.h"
#include "TRMacros.h"

UGPC_BR_SMG1::UGPC_BR_SMG1()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_1));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 12.0f;
	DeltaDmgAllyDirect = 0.0f;
}