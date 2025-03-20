// Copyright (C) 2025 by Haguk Kim


#include "TRHUDWidget.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "TRPlayerController.h"
#include "EquipSystem.h"
#include "WieldItem.h"

void UTRHUDWidget::SetTarget(AGameCharacter* Target)
{
	if (HudTarget)
	{
		HudTarget->UnbindHUD();
		HudTarget = nullptr;
	}
	Target->BindHUD(this);
	HudTarget = Target;
	UpdateAll();
}

void UTRHUDWidget::UpdateAmmo(int32 ClientAmmoPrediction)
{
	if (!AmmoLeftText) return;

	AFPSCharacter* HudFPSTarget = Cast<AFPSCharacter>(HudTarget);
	if (!IsValid(HudFPSTarget))
	{
		AmmoLeftText->SetText(FText::FromString(""));
	}
	else
	{
		if (ClientAmmoPrediction < 0)
		{
			AmmoLeftText->SetText(FText::FromString(HudFPSTarget->Host_GetCurrWeaponAmmoLeft()));
		}
		else
		{
			AmmoLeftText->SetText(FText::FromString(FString::Printf(TEXT("%d"), ClientAmmoPrediction)));
		}
	}
}

void UTRHUDWidget::UpdateSlot()
{
	if (!SlotText) return;

	if (!IsValid(HudTarget) || !HudTarget->EquipSystem)
	{
		SlotText->SetText(FText::FromString(""));
	}
	else
	{
		AWieldItem* SlotItem = HudTarget->EquipSystem->GetCurrWeaponActor();
		if (!IsValid(SlotItem))
		{
			SlotText->SetText(FText::FromString(""));
		}
		else
		{
			SlotText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *SlotItem->GetInvObject()->GetInvObjName())));
		}
	}
}

void UTRHUDWidget::UpdateDungeonDepth()
{
	if (!DungeonDepthText) return;
	UWorld* World = GetWorld();
	if (!World) return;

	ATRPlayerController* PC = Cast<ATRPlayerController>(GetGameInstance() ? GetGameInstance()->GetFirstLocalPlayerController() : nullptr);
	if (!IsValid(PC))
	{
		DungeonDepthText->SetText(FText::FromString(""));
	}
	else
	{
		DungeonDepthText->SetText(FText::FromString(FString::Printf(TEXT("%d"), PC->Local_GetCurrDungeonDepth())));
	}
}

void UTRHUDWidget::UpdateDungeonTimer()
{
	if (!WB_DungeonTimer) return;
	WB_DungeonTimer->Update();
}

void UTRHUDWidget::UpdateHealthBar()
{
	if (!WB_HealthBar) return;
	WB_HealthBar->Update(HudTarget);
}

void UTRHUDWidget::UpdateExp()
{
	if (!WB_LevelExpBar) return;
	WB_LevelExpBar->Update(HudTarget);
}

void UTRHUDWidget::UpdateShards()
{
	if (!WB_Shards) return;
	WB_Shards->Update(HudTarget);
}

void UTRHUDWidget::UpdateAll()
{
	UpdateAmmo();
	UpdateSlot();
	UpdateDungeonDepth();
	UpdateDungeonTimer();
	UpdateHealthBar();
	UpdateExp();
	UpdateShards();
}

