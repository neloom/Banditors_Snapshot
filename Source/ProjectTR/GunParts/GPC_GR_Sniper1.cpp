// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Sniper1.h"
#include "TRMacros.h"

UGPC_GR_Sniper1::UGPC_GR_Sniper1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_SNIPER_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.0f;
	DeltaProjInitialSpeed = 2000.0f;
	DeltaProjMaxSpeed = 2000.0f;
}