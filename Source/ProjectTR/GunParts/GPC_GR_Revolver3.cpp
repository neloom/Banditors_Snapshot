// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Revolver3.h"
#include "TRMacros.h"

UGPC_GR_Revolver3::UGPC_GR_Revolver3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_REVOLVER_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 2.0f;
	DeltaProjInitialSpeed = 1300.0f;
	DeltaProjMaxSpeed = 1300.0f;
}