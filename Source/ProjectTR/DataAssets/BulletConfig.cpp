// Copyright (C) 2024 by Haguk Kim


#include "BulletConfig.h"

FBulletData UBulletConfig::SearchBulletFromEnum(EBulletReference BulletRef) const
{
	switch (BulletRef)
	{
	case EBulletReference::EBU_NULL:
		return FBulletData();
	case EBulletReference::EBU_DefaultBullet:
		return DefaultBulletStruct;
	default:
		UE_LOG(LogTemp, Error, TEXT("SearchBulletFromEnum - Unable to find enum! %d"), BulletRef);
		return FBulletData();
	}
	return FBulletData();
}
