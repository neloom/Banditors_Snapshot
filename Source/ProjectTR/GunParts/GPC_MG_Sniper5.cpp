// Copyright (C) 2024 by Haguk Kim


#include "GPC_MG_Sniper5.h"
#include "TRMacros.h"

UGPC_MG_Sniper5::UGPC_MG_Sniper5()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MAGAZINE_SNIPER_5));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideFireMode = true;
	FireModeValue = EWeaponFireMode::WFM_AUTO;
}


