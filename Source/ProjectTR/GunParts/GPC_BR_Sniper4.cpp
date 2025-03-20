// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Sniper4.h"
#include "TRMacros.h"

UGPC_BR_Sniper4::UGPC_BR_Sniper4()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SNIPER_4));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 67.0f;
	DeltaDmgAllyDirect = 67.0f;
}