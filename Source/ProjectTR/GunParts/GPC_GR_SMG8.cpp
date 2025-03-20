// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_SMG8.h"
#include "TRMacros.h"

UGPC_GR_SMG8::UGPC_GR_SMG8()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_SMG_8));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 1.0f;
	DeltaProjInitialSpeed = 3000.0f;
	DeltaProjMaxSpeed = 3000.0f;
}