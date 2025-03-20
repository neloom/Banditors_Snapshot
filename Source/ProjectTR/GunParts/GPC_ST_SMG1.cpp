// Copyright (C) 2024 by Haguk Kim


#include "GPC_ST_SMG1.h"
#include "TRMacros.h"

UGPC_ST_SMG1::UGPC_ST_SMG1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_STOCK_SMG_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideHasDmgDistFallOff = true;
	bHasDmgDistFallOffValue = true;
	DeltaDmgDistFallOffMult = -0.3f;
}