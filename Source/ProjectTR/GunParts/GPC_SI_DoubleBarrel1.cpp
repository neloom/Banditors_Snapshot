// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_DoubleBarrel1.h"
#include "TRMacros.h"

UGPC_SI_DoubleBarrel1::UGPC_SI_DoubleBarrel1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_DOUBLEBARREL_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

