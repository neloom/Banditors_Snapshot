// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_SMG7.h"
#include "TRMacros.h"

UGPC_GR_SMG7::UGPC_GR_SMG7()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_SMG_7));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 1.1f;
	DeltaProjInitialSpeed = 1200.0f;
	DeltaProjMaxSpeed = 1200.0f;
}