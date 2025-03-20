// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_Shotgun2.h"
#include "TRMacros.h"

UGPC_RC_Shotgun2::UGPC_RC_Shotgun2()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_SHOTGUN_2));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_HITSCAN;
	DeltaFireInterval = 0.70f;
}