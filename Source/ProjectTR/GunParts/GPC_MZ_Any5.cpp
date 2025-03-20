// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any5.h"
#include "TRMacros.h"

UGPC_MZ_Any5::UGPC_MZ_Any5()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_5));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

