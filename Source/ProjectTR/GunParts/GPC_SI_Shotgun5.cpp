// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Shotgun5.h"
#include "TRMacros.h"

UGPC_SI_Shotgun5::UGPC_SI_Shotgun5()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SHOTGUN_5));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

