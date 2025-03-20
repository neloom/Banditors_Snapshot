// Copyright (C) 2024 by Haguk Kim


#include "GPC_ST_DoubleBarrel2.h"
#include "TRMacros.h"

UGPC_ST_DoubleBarrel2::UGPC_ST_DoubleBarrel2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_STOCK_DOUBLEBARREL_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideHasDmgDistFallOff = true;
	bHasDmgDistFallOffValue = true;
	DeltaDmgDistFallOffMult = -0.7f;
}