// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Rifle2.h"
#include "TRMacros.h"

UGPC_GR_Rifle2::UGPC_GR_Rifle2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_RIFLE_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.3f;
	DeltaProjInitialSpeed = 3300.0f;
	DeltaProjMaxSpeed = 3300.0f;
}