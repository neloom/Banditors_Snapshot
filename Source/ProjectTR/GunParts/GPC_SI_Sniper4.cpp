// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Sniper4.h"
#include "TRMacros.h"

UGPC_SI_Sniper4::UGPC_SI_Sniper4()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SNIPER_4));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaDmgMultOnHead = 0.5f;
}

