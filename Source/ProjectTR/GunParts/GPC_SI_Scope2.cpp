// Copyright (C) 2024 by Haguk Kim


#include "GPC_SI_Scope2.h"
#include "TRMacros.h"

UGPC_SI_Scope2::UGPC_SI_Scope2()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_SIGHT_SCOPE_2));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);
}

