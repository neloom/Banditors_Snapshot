// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Shotgun3.h"
#include "TRMacros.h"

UGPC_GR_Shotgun3::UGPC_GR_Shotgun3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_SHOTGUN_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 3.0f;
	DeltaProjInitialSpeed = 2000.0f;
	DeltaProjMaxSpeed = 2000.0f;
}