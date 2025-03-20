// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_DoubleBarrel3.h"
#include "TRMacros.h"

UGPC_BR_DoubleBarrel3::UGPC_BR_DoubleBarrel3()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_DOUBLEBARREL_3));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 30.0f;
	DeltaDmgAllyDirect = -30.0f;
}