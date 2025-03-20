// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Shotgun3.h"
#include "TRMacros.h"

UGPC_SI_Shotgun3::UGPC_SI_Shotgun3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SHOTGUN_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

