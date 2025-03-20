// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG10.h"
#include "TRMacros.h"

UGPC_BR_SMG10::UGPC_BR_SMG10()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_10));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 39.0f;
	DeltaDmgAllyDirect = 29.0f;
}