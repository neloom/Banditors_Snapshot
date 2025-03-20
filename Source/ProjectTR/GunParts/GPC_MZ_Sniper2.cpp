// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Sniper2.h"
#include "TRMacros.h"

UGPC_MZ_Sniper2::UGPC_MZ_Sniper2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_SNIPER_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

