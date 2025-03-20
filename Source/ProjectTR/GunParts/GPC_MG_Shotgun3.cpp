// Copyright (C) 2024 by Haguk Kim


#include "GPC_MG_Shotgun3.h"
#include "TRMacros.h"

UGPC_MG_Shotgun3::UGPC_MG_Shotgun3()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MAGAZINE_SHOTGUN_3));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideFireMode = true;
	FireModeValue = EWeaponFireMode::WFM_SINGLE;
}

