// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_Shotgun3.h"
#include "TRMacros.h"

UGPC_RC_Shotgun3::UGPC_RC_Shotgun3()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_SHOTGUN_3));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_PROJECTILE;
	DeltaFireInterval = 0.65f;
}