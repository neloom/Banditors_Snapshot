// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_Pistol1.h"
#include "TRMacros.h"

UGPC_RC_Pistol1::UGPC_RC_Pistol1()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_PISTOL_1));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_PROJECTILE;
	DeltaFireInterval = -0.33f;
}