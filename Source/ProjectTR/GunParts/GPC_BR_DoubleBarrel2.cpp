// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_DoubleBarrel2.h"
#include "TRMacros.h"

UGPC_BR_DoubleBarrel2::UGPC_BR_DoubleBarrel2()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_DOUBLEBARREL_2));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 50.0f;
	DeltaDmgAllyDirect = 35.0f;
}