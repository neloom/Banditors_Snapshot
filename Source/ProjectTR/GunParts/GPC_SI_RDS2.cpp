// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_RDS2.h"
#include "TRMacros.h"

UGPC_SI_RDS2::UGPC_SI_RDS2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_RDS_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

