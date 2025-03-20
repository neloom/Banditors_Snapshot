// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AnimConfig.generated.h"

/**
 * AnimConfig는 기본적인 틀만 제공하며, 실제 바인딩은 에셋 에디터에서 이루어진다.
 * 때문에 애님클래스의 사용 목적에 따라 다른 애니메이션을 바인딩할 수도 있으며,
 * 따라서 애니메이션을 재생하기 전에 Null체크를 해주는 것과, 사용 스켈레톤에 대응되는 적절한 애니메이션을 바인딩 시켜주는 것이 중요하다.
 */
UCLASS()
class PROJECTTR_API UAnimConfig : public UDataAsset
{
	GENERATED_BODY()

/* Montages */
public:
	/* Item Deployment */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_DeployRifle = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_DeployPistol = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_GenericUnequip = nullptr;

	/* Melee */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_MeleeAtk1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_MeleeAtk2 = nullptr;

	/* Ranged */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_RangedAtk1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_RangedAtk2 = nullptr;

/* AnimClass */
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> UnarmedAnimClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> LightWeaponAnimClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> HeavyWeaponAnimClass = nullptr;
};
