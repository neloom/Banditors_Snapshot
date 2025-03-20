// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Sniper6.h"
#include "TRMacros.h"

UGPC_BR_Sniper6::UGPC_BR_Sniper6()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_SNIPER_6));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 42.0f;
	DeltaDmgAllyDirect = -15.0f;
}