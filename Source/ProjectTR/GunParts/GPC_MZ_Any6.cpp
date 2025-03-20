// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any6.h"
#include "TRMacros.h"

UGPC_MZ_Any6::UGPC_MZ_Any6()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_6));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

