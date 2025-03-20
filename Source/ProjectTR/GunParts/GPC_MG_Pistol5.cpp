// Copyright (C) 2024 by Haguk Kim


#include "GPC_MG_Pistol5.h"
#include "TRMacros.h"

UGPC_MG_Pistol5::UGPC_MG_Pistol5()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT(SM_MAGAZINE_PISTOL_5));
	UStaticMesh* Asset = MeshAsset.Object;
	SetupMeshComp(Asset, nullptr);

	// TEMP TODO FIXME
	bOverrideFireMode = true;
	FireModeValue = EWeaponFireMode::WFM_SINGLE;
}

