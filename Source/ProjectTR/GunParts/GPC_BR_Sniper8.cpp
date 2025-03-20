// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Sniper8.h"
#include "TRMacros.h"

UGPC_BR_Sniper8::UGPC_BR_Sniper8()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SNIPER_8));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 13.0f;
	DeltaDmgAllyDirect = 3.0f;
}