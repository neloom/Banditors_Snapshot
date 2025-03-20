// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any7.h"
#include "TRMacros.h"

UGPC_MZ_Any7::UGPC_MZ_Any7()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_7));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

