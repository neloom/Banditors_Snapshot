// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "InputConfig.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> MoveInputAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> LookInputAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> JumpInputAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> AttackAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> Attack2Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> AttackStopAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> Attack2StopAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> InventoryAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> DuckAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> SlideAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> TauntAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> RollAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> SwitchTo0Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> SwitchTo1Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> SwitchTo2Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> SwitchTo3Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> ToggleViewPerspectiveAction;
};
