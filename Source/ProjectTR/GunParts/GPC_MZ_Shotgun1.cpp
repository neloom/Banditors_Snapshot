// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Shotgun1.h"
#include "TRMacros.h"

UGPC_MZ_Shotgun1::UGPC_MZ_Shotgun1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_SHOTGUN_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

