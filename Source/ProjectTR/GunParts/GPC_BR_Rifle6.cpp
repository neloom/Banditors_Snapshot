// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Rifle6.h"
#include "TRMacros.h"

UGPC_BR_Rifle6::UGPC_BR_Rifle6()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_6));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 33.0f;
	DeltaDmgAllyDirect = 0.0f;
}