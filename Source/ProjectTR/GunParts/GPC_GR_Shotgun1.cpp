// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Shotgun1.h"
#include "TRMacros.h"

UGPC_GR_Shotgun1::UGPC_GR_Shotgun1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_SHOTGUN_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 1.6f;
	DeltaProjInitialSpeed = 1700.0f;
	DeltaProjMaxSpeed = 1700.0f;
}