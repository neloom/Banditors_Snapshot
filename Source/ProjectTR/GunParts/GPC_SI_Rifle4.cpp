// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Rifle4.h"
#include "TRMacros.h"

UGPC_SI_Rifle4::UGPC_SI_Rifle4()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_RIFLE_4));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaDmgMultOnHead = 0.3f;
}

