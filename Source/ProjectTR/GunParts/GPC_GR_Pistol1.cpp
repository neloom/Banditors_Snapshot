// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Pistol1.h"
#include "TRMacros.h"

UGPC_GR_Pistol1::UGPC_GR_Pistol1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_PISTOL_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.5f;
	DeltaProjInitialSpeed = 1000.0f;
	DeltaProjMaxSpeed = 1000.0f;
}