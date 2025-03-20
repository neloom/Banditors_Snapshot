// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Pistol2.h"
#include "TRMacros.h"

UGPC_GR_Pistol2::UGPC_GR_Pistol2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_PISTOL_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.8f;
	DeltaProjInitialSpeed = 1500.0f;
	DeltaProjMaxSpeed = 1500.0f;
}