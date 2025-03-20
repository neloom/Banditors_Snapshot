// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_SMG4.h"
#include "TRMacros.h"

UGPC_GR_SMG4::UGPC_GR_SMG4()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_SMG_4));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.05f;
	DeltaProjInitialSpeed = 4600.0f;
	DeltaProjMaxSpeed = 4600.0f;
}