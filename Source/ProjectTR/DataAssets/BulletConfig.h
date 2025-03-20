// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TRExplosion.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BulletConfig.generated.h"

// 불릿 데이터는 오직 게임 로직에 영향을 주지 않는 값들만을 가지고 있어야 한다
USTRUCT(BlueprintType)
struct FBulletData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* TracerFx = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HitCharFx = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HitObjFx = nullptr;
};


UENUM(BlueprintType)
enum class EBulletReference : uint8
{
	EBU_NULL,
	EBU_DefaultBullet,
};

/**
 * 
 */
UCLASS()
class PROJECTTR_API UBulletConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Enum 기반 에셋 서칭
	FBulletData SearchBulletFromEnum(EBulletReference BulletRef) const;

/* Bullets */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBulletData DefaultBulletStruct;
};
