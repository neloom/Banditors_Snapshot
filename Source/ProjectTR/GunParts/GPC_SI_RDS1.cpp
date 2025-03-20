// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_RDS1.h"
#include "TRMacros.h"

UGPC_SI_RDS1::UGPC_SI_RDS1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_RDS_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

