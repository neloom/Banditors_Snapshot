// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "FxConfig.generated.h"

UENUM(BlueprintType)
enum class ENiagaraReference : uint8
{
	ENR_NULL,

	// Muzzle flash
	ENR_MZF_Physical_1,
	ENR_MZF_Physical_2,
	ENR_MZF_Physical_3,
	ENR_MZF_Physical_Shotgun_4,
	ENR_MZF_Plasma_1,
	ENR_MZF_Purple_1,
	ENR_MZF_Energy_1,
	ENR_MZF_Energy_2,
	ENR_MZF_Energy_3,
	ENR_MZF_Laser_1,
	ENR_MZF_Laser_2,

	// Hit impact
	ENR_HIT_BoxFlash_1,
	ENR_HIT_Physical_1,
	ENR_HIT_Physical_2,
	ENR_HIT_Physical_3,
	ENR_HIT_Physical_Shotgun_4,
	ENR_HIT_Plasma_1,
	ENR_HIT_Purple_1,
	ENR_HIT_Energy_1,
	ENR_HIT_Energy_2,
	ENR_HIT_Energy_3,
	ENR_HIT_Laser_1,
	ENR_HIT_Laser_2,

	// Bullet trail
	ENR_BLT_Default,

	// Shell ejection
	ENR_SEJ_Default,

	// Explosion
	ENR_EXP_Default,
};

/**
 * 
 */
UCLASS()
class PROJECTTR_API UFxConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// 네임태그 기반 에셋 서칭
	// NOTE: 
	// GunItem의 GPC 레플리케이션과, 해당 GPC가 사용할 VFX 간의 의존관계를 없애고자 Enum을 사용해 사용할 VFX에 대한 정보를 저장함
	UNiagaraSystem* SearchNiagaraFromEnum(ENiagaraReference NiagaraRef) const;

/* Gun VFXs */
/* Muzzle flashes */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Physical_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Physical_2 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Physical_3 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Physical_Shotgun_4 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Plasma_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Purple_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Energy_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Energy_2 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Energy_3 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Laser_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MZF_Laser_2 = nullptr;

	/* Hit Impact */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_BoxFlash_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Physical_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Physical_2 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Physical_3 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Physical_Shotgun_4 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Plasma_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Purple_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Energy_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Energy_2 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Energy_3 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Laser_1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* HIT_Laser_2 = nullptr;

	/* Hit Decal */
public:
	// TODO

/* Bullet Trail */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* BLT_Default = nullptr;

/* Shell Ejection */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* SEJ_Default = nullptr;

/* Explosion */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* EXP_Default = nullptr;
};
