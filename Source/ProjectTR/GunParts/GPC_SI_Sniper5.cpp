// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Sniper5.h"
#include "TRMacros.h"

UGPC_SI_Sniper5::UGPC_SI_Sniper5()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SNIPER_5));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaDmgMultOnHead = 1.2f;
}

