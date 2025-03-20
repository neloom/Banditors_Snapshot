// Copyright (C) 2024 by Haguk Kim


#include "GPC_BR_Revolver2.h"
#include "TRMacros.h"

UGPC_BR_Revolver2::UGPC_BR_Revolver2()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_REVOLVER_2));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	DeltaDmgEnemyDirect = 0.0f;
	DeltaDmgAllyDirect = -10.0f;
}