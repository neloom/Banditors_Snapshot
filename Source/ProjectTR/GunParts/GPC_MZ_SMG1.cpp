// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_SMG1.h"
#include "TRMacros.h"

UGPC_MZ_SMG1::UGPC_MZ_SMG1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_SMG_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

