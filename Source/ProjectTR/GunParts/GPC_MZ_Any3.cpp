// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any3.h"
#include "TRMacros.h"

UGPC_MZ_Any3::UGPC_MZ_Any3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

