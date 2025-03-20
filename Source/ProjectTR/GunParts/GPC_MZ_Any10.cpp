// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any10.h"
#include "TRMacros.h"

UGPC_MZ_Any10::UGPC_MZ_Any10()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_10));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

