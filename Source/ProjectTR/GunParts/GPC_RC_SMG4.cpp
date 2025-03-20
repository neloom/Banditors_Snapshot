// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_SMG4.h"
#include "TRMacros.h"

UGPC_RC_SMG4::UGPC_RC_SMG4()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_SMG_4));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_PROJECTILE;
	DeltaFireInterval = -0.7f;
}