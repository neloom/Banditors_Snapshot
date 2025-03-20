// Copyright (C) 2024 by Haguk Kim


#include "GPC_ST_Shotgun1.h"
#include "TRMacros.h"

UGPC_ST_Shotgun1::UGPC_ST_Shotgun1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_STOCK_SHOTGUN_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideHasDmgDistFallOff = true;
	bHasDmgDistFallOffValue = true;
	DeltaDmgDistFallOffMult = -0.65f;
}