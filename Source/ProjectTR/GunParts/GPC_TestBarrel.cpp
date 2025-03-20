// Copyright (C) 2024 by Haguk Kim


#include "GPC_TestBarrel.h"
#include "TRMacros.h"

UGPC_TestBarrel::UGPC_TestBarrel()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_BARREL_RIFLE_5));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);
}