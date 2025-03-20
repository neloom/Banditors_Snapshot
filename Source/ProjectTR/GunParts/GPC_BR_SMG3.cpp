// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG3.h"
#include "TRMacros.h"

UGPC_BR_SMG3::UGPC_BR_SMG3()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_3));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 18.0f;
	DeltaDmgAllyDirect = 0.0f;
}