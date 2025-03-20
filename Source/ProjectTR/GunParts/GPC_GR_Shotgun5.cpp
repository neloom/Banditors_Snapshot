// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Shotgun5.h"
#include "TRMacros.h"

UGPC_GR_Shotgun5::UGPC_GR_Shotgun5()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_SHOTGUN_5));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 1.5f;
	DeltaProjInitialSpeed = 1500.0f;
	DeltaProjMaxSpeed = 1500.0f;
}