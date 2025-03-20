// Copyright (C) 2024 by Haguk Kim


#include "GPC_GR_Rifle8.h"
#include "TRMacros.h"

UGPC_GR_Rifle8::UGPC_GR_Rifle8()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_GRIP_RIFLE_8));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	DeltaRecoilOffsetRange = 0.5f;
	DeltaProjInitialSpeed = 200.0f;
	DeltaProjMaxSpeed = 200.0f;
}