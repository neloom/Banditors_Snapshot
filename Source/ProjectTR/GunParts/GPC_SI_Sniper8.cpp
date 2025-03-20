// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Sniper8.h"
#include "TRMacros.h"

UGPC_SI_Sniper8::UGPC_SI_Sniper8()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SNIPER_8));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaDmgMultOnHead = 2.0f;
}

