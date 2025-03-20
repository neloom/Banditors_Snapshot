// Copyright (C) 2024 by Haguk Kim


#include "GPC_MG_Rifle3.h"
#include "TRMacros.h"

UGPC_MG_Rifle3::UGPC_MG_Rifle3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MAGAZINE_RIFLE_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideFireMode = true;
	FireModeValue = EWeaponFireMode::WFM_AUTO;
}


