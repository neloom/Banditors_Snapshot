// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Rifle6.h"
#include "TRMacros.h"

UGPC_SI_Rifle6::UGPC_SI_Rifle6()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_RIFLE_6));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

