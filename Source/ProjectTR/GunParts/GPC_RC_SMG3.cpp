// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_SMG3.h"
#include "TRMacros.h"

UGPC_RC_SMG3::UGPC_RC_SMG3()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_SMG_3));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_HITSCAN;
	DeltaFireInterval = -0.91f;
}