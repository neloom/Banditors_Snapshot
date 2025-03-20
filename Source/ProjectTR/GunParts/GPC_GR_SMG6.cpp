// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_SMG6.h"
#include "TRMacros.h"

UGPC_GR_SMG6::UGPC_GR_SMG6()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_SMG_6));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.7f;
	DeltaProjInitialSpeed = 1000.0f;
	DeltaProjMaxSpeed = 1000.0f;
}