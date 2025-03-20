// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseProjectile.h"
#include "ProjectileConfig.generated.h"

UENUM(BlueprintType)
enum class EProjectileReference : uint8
{
	EPR_NULL,
	EPR_DefaultProj,
};

/**
 * 
 */
UCLASS()
class PROJECTTR_API UProjectileConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// Enum 기반 에셋 서칭
	TSubclassOf<ABaseProjectile> SearchProjectileFromEnum(EProjectileReference ProjRef) const;

/* Projectiles */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ABaseProjectile> DefaultProjClass = nullptr;
};
