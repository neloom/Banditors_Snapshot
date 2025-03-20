// Copyright (C) 2024 by Haguk Kim


#include "GPC_ST_Pistol1.h"
#include "TRMacros.h"

UGPC_ST_Pistol1::UGPC_ST_Pistol1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_STOCK_PISTOL_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideHasDmgDistFallOff = true;
	bHasDmgDistFallOffValue = true;
	DeltaDmgDistFallOffMult = -0.3f;
}