// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Revolver2.h"
#include "TRMacros.h"

UGPC_GR_Revolver2::UGPC_GR_Revolver2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_REVOLVER_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.0f;
	DeltaProjInitialSpeed = 2500.0f;
	DeltaProjMaxSpeed = 2500.0f;
}