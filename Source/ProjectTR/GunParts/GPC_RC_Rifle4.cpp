// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_Rifle4.h"
#include "TRMacros.h"

UGPC_RC_Rifle4::UGPC_RC_Rifle4()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_RIFLE_4));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_HITSCAN;
	DeltaFireInterval = -0.87f;
}