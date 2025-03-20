// Copyright (C) 2024 by Haguk Kim


#include "GPC_ST_Rifle10.h"
#include "TRMacros.h"

UGPC_ST_Rifle10::UGPC_ST_Rifle10()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_STOCK_RIFLE_10));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideHasDmgDistFallOff = true;
	bHasDmgDistFallOffValue = true;
	DeltaDmgDistFallOffMult = -0.48f;
}