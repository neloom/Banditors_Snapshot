// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Rifle3.h"
#include "TRMacros.h"

UGPC_GR_Rifle3::UGPC_GR_Rifle3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_RIFLE_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.0f;
	DeltaProjInitialSpeed = 4000.0f;
	DeltaProjMaxSpeed = 4000.0f;
}