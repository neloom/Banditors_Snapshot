// Copyright (C) 2024 by Haguk Kim


#include "GPC_ST_SMG3.h"
#include "TRMacros.h"

UGPC_ST_SMG3::UGPC_ST_SMG3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_STOCK_SMG_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideHasDmgDistFallOff = true;
	bHasDmgDistFallOffValue = false;
}