// Copyright (C) 2024 by Haguk Kim


#include "FirstPersonMeshComponent.h"

UFirstPersonMeshComponent::UFirstPersonMeshComponent()
{
	SetOnlyOwnerSee(true);
	bCastDynamicShadow = false; // 캐릭터 메쉬와 일체감을 주기 위함
	CastShadow = false;
}
