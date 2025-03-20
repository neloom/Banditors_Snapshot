// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Rifle6.h"
#include "TRMacros.h"

UGPC_GR_Rifle6::UGPC_GR_Rifle6()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_RIFLE_6));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.15f;
	DeltaProjInitialSpeed = 600.0f;
	DeltaProjMaxSpeed = 600.0f;
}