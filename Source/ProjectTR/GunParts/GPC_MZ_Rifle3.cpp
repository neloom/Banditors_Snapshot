// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Rifle3.h"
#include "TRMacros.h"

UGPC_MZ_Rifle3::UGPC_MZ_Rifle3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_RIFLE_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

