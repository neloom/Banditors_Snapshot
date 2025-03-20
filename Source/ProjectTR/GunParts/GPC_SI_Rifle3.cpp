// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Rifle3.h"
#include "TRMacros.h"

UGPC_SI_Rifle3::UGPC_SI_Rifle3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_RIFLE_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

