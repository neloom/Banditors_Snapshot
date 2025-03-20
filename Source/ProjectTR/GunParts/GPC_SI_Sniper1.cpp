// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Sniper1.h"
#include "TRMacros.h"

UGPC_SI_Sniper1::UGPC_SI_Sniper1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SNIPER_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaDmgMultOnHead = 1.1f;
}

