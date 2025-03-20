// Copyright (C) 2024 by Haguk Kim


#include "GPC_ST_Rifle9.h"
#include "TRMacros.h"

UGPC_ST_Rifle9::UGPC_ST_Rifle9()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_STOCK_RIFLE_9));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideHasDmgDistFallOff = true;
	bHasDmgDistFallOffValue = true;
	DeltaDmgDistFallOffMult = -0.23f;
}