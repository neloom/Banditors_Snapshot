// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Sniper5.h"
#include "TRMacros.h"

UGPC_BR_Sniper5::UGPC_BR_Sniper5()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SNIPER_5));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 38.0f;
	DeltaDmgAllyDirect = 38.0f;
}