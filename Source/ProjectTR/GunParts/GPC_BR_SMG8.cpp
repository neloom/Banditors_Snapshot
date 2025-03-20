// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG8.h"
#include "TRMacros.h"

UGPC_BR_SMG8::UGPC_BR_SMG8()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_8));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = -50.0f;
	DeltaDmgAllyDirect = 0.0f;
}