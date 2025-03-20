// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Shotgun2.h"
#include "TRMacros.h"

UGPC_SI_Shotgun2::UGPC_SI_Shotgun2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SHOTGUN_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

