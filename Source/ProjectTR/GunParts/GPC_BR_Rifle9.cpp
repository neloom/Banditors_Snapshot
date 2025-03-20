// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Rifle9.h"
#include "TRMacros.h"

UGPC_BR_Rifle9::UGPC_BR_Rifle9()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_9));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 5.0f;
	DeltaDmgAllyDirect = 0.0f;
}