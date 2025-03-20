// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Shotgun4.h"
#include "TRMacros.h"

UGPC_SI_Shotgun4::UGPC_SI_Shotgun4()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SHOTGUN_4));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

