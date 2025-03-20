// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_Sniper3.h"
#include "TRMacros.h"

UGPC_RC_Sniper3::UGPC_RC_Sniper3()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_SNIPER_3));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_HITSCAN;
	DeltaFireInterval = -0.45f;
}