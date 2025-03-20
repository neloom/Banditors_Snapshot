// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Sniper3.h"
#include "TRMacros.h"

UGPC_SI_Sniper3::UGPC_SI_Sniper3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SNIPER_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaDmgMultOnHead = 0.8f;
}

