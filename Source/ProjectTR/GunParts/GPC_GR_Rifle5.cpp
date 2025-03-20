// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Rifle5.h"
#include "TRMacros.h"

UGPC_GR_Rifle5::UGPC_GR_Rifle5()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_RIFLE_5));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.0f;
	DeltaProjInitialSpeed = 2700.0f;
	DeltaProjMaxSpeed = 2700.0f;
}