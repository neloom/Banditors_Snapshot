// Copyright (C) 2024 by Haguk Kim


#include "GPC_RC_DoubleBarrel1.h"
#include "TRMacros.h"

UGPC_RC_DoubleBarrel1::UGPC_RC_DoubleBarrel1()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT(SK_RECEIVER_DOUBLEBARREL_1));
	USkeletalMesh* Asset = MeshAsset.Object;
	SetupMeshComp(nullptr, Asset);

	// TEMP TODO FIXME
	bOverrideGunType = true;
	GunTypeValue = EWeaponFireType::WFT_PROJECTILE;
	DeltaFireInterval = -0.9f;
}