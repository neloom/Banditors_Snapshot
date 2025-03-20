// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Rifle8.h"
#include "TRMacros.h"

UGPC_BR_Rifle8::UGPC_BR_Rifle8()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_8));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 0.0f;
	DeltaDmgAllyDirect = -30.0f;
}