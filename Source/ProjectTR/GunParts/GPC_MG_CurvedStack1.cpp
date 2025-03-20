// Copyright (C) 2024 by Haguk Kim


#include "GPC_MG_CurvedStack1.h"
#include "TRMacros.h"

UGPC_MG_CurvedStack1::UGPC_MG_CurvedStack1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MAGAZINE_CURVED_STACK_1));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideFireMode = true;
	FireModeValue = EWeaponFireMode::WFM_SINGLE;
}

