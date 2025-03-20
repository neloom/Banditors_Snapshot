// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_SMG2.h"
#include "TRMacros.h"

UGPC_MZ_SMG2::UGPC_MZ_SMG2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_SMG_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

