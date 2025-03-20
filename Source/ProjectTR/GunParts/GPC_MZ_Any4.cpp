// Copyright (C) 2024 by Haguk Kim


#include "GPC_MZ_Any4.h"
#include "TRMacros.h"

UGPC_MZ_Any4::UGPC_MZ_Any4()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MUZZLE_ANY_4));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

