// Copyright (C) 2025 by Haguk Kim


#include "AudioConfig.h"

USoundCue* UAudioConfig::SearchAudioFromEnum(EGunshotAudioReference AudioRef) const
{
	switch (AudioRef)
	{
	case EGunshotAudioReference::EGR_NULL:
		return nullptr;
	case EGunshotAudioReference::EGR_MACHINEGUN_A_1:
		return MachineGun_A_Fire_1;
	case EGunshotAudioReference::EGR_MACHINEGUN_A_2:
		return MachineGun_A_Fire_2;
	case EGunshotAudioReference::EGR_MACHINEGUN_A_3:
		return MachineGun_A_Fire_3;
	case EGunshotAudioReference::EGR_MACHINEGUN_A_4:
		return MachineGun_A_Fire_4;
	case EGunshotAudioReference::EGR_MACHINEGUN_A_5:
		return MachineGun_A_Fire_5;
	case EGunshotAudioReference::EGR_MACHINEGUN_A_6:
		return MachineGun_A_Fire_6;
	case EGunshotAudioReference::EGR_MACHINEGUN_A_7:
		return MachineGun_A_Fire_7;
	case EGunshotAudioReference::EGR_MACHINEGUN_A_8:
		return MachineGun_A_Fire_8;
	///////////// TODO
	default:
		UE_LOG(LogTemp, Error, TEXT("SearchAudioFromEnum - Unable to find EGunshotAudioReference enum! %d"), AudioRef);
		return nullptr;
	}
	return nullptr;
}

USoundCue* UAudioConfig::SearchAudioFromEnum(EGunMiscAudioReference AudioRef) const
{
	switch (AudioRef)
	{
	case EGunMiscAudioReference::EGM_NULL:
		return nullptr;
	case EGunMiscAudioReference::EGM_AMMOEMPTY_1:
		return AmmoEmpty_1;
	//////////////// TODO
	default:
		UE_LOG(LogTemp, Error, TEXT("SearchAudioFromEnum - Unable to find EGunMiscAudioReference enum! %d"), AudioRef);
		return nullptr;
	}
	return nullptr;
}
