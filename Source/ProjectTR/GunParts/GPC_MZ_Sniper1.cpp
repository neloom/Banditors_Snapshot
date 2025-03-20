// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Sniper1.h"
#include "TRMacros.h"

UGPC_MZ_Sniper1::UGPC_MZ_Sniper1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_SNIPER_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

