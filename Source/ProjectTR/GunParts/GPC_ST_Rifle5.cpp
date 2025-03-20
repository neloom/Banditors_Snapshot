// Copyright (C) 2024 by Haguk Kim


#include "GPC_ST_Rifle5.h"
#include "TRMacros.h"

UGPC_ST_Rifle5::UGPC_ST_Rifle5()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_STOCK_RIFLE_5));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideHasDmgDistFallOff = true;
	bHasDmgDistFallOffValue = true;
	DeltaDmgDistFallOffMult = -0.24f;
}