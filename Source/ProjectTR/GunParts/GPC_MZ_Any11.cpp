// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any11.h"
#include "TRMacros.h"

UGPC_MZ_Any11::UGPC_MZ_Any11()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_11));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

