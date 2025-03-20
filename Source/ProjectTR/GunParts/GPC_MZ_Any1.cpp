// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any1.h"
#include "TRMacros.h"

UGPC_MZ_Any1::UGPC_MZ_Any1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

