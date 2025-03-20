// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_SMG6.h"
#include "TRMacros.h"

UGPC_BR_SMG6::UGPC_BR_SMG6()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SMG_6));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 21.0f;
	DeltaDmgAllyDirect = 21.0f;
}