// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any8.h"
#include "TRMacros.h"

UGPC_MZ_Any8::UGPC_MZ_Any8()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_8));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

