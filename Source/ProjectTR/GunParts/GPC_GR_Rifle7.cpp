// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Rifle7.h"
#include "TRMacros.h"

UGPC_GR_Rifle7::UGPC_GR_Rifle7()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_RIFLE_7));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.4f;
	DeltaProjInitialSpeed = 3600.0f;
	DeltaProjMaxSpeed = 3600.0f;
}