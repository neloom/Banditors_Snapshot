// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_Pistol2.h"
#include "TRMacros.h"

UGPC_RC_Pistol2::UGPC_RC_Pistol2()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_PISTOL_2));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_HITSCAN;
	DeltaFireInterval = -0.75f;
}