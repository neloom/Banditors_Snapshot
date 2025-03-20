// Copyright (C) 2024 by Haguk Kim


#include "TRShrine.h"
#include "TRMacros.h"
#include "EngineUtils.h"
#include "GameCharacter.h"
#include "TRUtils.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ATRShrine::ATRShrine()
{
	GemLightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("GemLight"));
	GemLightComp->SetupAttachment(RootComponent);
}

void ATRShrine::BeginPlay()
{
	Super::BeginPlay();

	InitMaterials();

	if (HasAuthority())
	{
		Server_SetShrineState(bDefaultUsability);
	}
	else
	{
		// 클라에서 최초 디폴트 상태를 설정하는 멀티캐스트는 전달되지 않을 수 있기 때문에, 이는 수동으로 직접 호출해주어야 한다
		Multicast_SetShrineState(bDefaultUsability);
	}
}

void ATRShrine::OnMuzzleTriggered(AGameCharacter* TriggeredBy)
{
	if (!TriggeredBy)
	{
		UE_LOG(LogTemp, Error, TEXT("OnMuzzleTriggered - Triggered by invalid character!"));
		return;
	}
	if (!HasAuthority()) return;
	if (!bCurrentUsability) return;
	OnShrineTriggered(TriggeredBy);
}

void ATRShrine::OnShrineTriggered(AGameCharacter* TriggeredBy)
{
	if (!TriggeredBy) return;
	if (bUseOnce)
	{
		Server_SetShrineState(false);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("OnShrineTriggered - Invalid world!"));
		return;
	}

	if (StatusEffectApplyType == EShrineAffectType::SAT_None)
	{
		return;
	}

	TSet<AGameCharacter*> StatEffTargets;
	switch (StatusEffectApplyType)
	{
	case EShrineAffectType::SAT_InvokerOnly:
	case EShrineAffectType::SAT_InvokerAndAllies:
	case EShrineAffectType::SAT_InvokerAndNearbyAllies:
		StatEffTargets.Add(TriggeredBy);
		break;
	default:
		break;
	}

	for (TActorIterator<AGameCharacter> It(World); It; ++It)
	{
		AGameCharacter* Target = *It;
		if (!IsValid(Target)) continue;

		bool bWithinDistance = this->GetDistanceTo(Target) <= TargetAffectDistance;
		bool bIsAllyWithInvoker = TRUtils::IsAllyWith(TriggeredBy, Target);

		switch (StatusEffectApplyType)
		{
			case EShrineAffectType::SAT_InvokerAndNearbyAllies:
			{
				if (bWithinDistance && bIsAllyWithInvoker)
				{
					StatEffTargets.Add(Target);
				}
				break;
			}
			case EShrineAffectType::SAT_InvokerAndAllies:
			{
				if (bIsAllyWithInvoker)
				{
					StatEffTargets.Add(Target);
				}
				break;
			}
			case EShrineAffectType::SAT_NearbyEnemies:
			{
				if (bWithinDistance && !bIsAllyWithInvoker)
				{
					StatEffTargets.Add(Target);
				}
				break;
			}
			case EShrineAffectType::SAT_AllEnemies:
			{
				if (!bIsAllyWithInvoker)
				{
					StatEffTargets.Add(Target);
				}
				break;
			}
			default:
			{
				UE_LOG(LogTemp, Error, TEXT("OnShrineTriggered - Something went wrong!"));
				break;
			}
		}
	}

	for (AGameCharacter* StatEffTarget : StatEffTargets)
	{
		if (StatEffTarget)
		{
			for (const FStatEffectGenInfo& StatEffect : StatEffAppliedToTargets)
			{
				StatEffTarget->Server_GenerateAndAddStatEffect(StatEffect, TriggeredBy/* 부여자는 최초 트리거 시킨 대상으로 취급*/);
			}
		}
	}
	TR_PRINT("ATRShrine::OnShrineTriggered");
}

void ATRShrine::ShrineLogic_Implementation(AGameCharacter* TriggeredBy)
{
	// 필요 시 따로 오버라이드
}

void ATRShrine::InitMaterials()
{
	// 중앙 보석 부분을 다이나믹 매터리얼로 교체
	if (MeshComponent)
	{
		int32 MaterialCount = MeshComponent->GetNumMaterials();
		if (GemMaterialIndex < 0 || GemMaterialIndex >= MaterialCount)
		{
			UE_LOG(LogTemp, Error, TEXT("ATRShrine::InitMaterials - Invalid material index. Please check!"));
			return;
		}

		UMaterialInterface* Material = MeshComponent->GetMaterial(GemMaterialIndex);
		if (Material)
		{
			GemMaterialInst = UMaterialInstanceDynamic::Create(Material, this);
			MeshComponent->SetMaterial(GemMaterialIndex, GemMaterialInst);
		}
	}
}

void ATRShrine::Server_SetShrineState(bool bUsable)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_SetShrineState - called by client"));
		return;
	}
	bCurrentUsability = bUsable;
	Multicast_SetShrineState(bUsable);
}

void ATRShrine::Multicast_SetShrineState_Implementation(bool bUsable)
{
	if (bUsable)
	{
		SetMatToUsable();
		SetLightToUsable();
	}
	else
	{
		SetMatToUnusable();
		SetLightToUnusable();
	}
}

void ATRShrine::SetMatToUsable()
{
	if (GemMaterialInst)
	{
		GemMaterialInst->SetVectorParameterValue(FName(BaseColorParamName), BaseColorUsable);
		GemMaterialInst->SetVectorParameterValue(FName(LightColorParamName), LightColorUsable);
		GemMaterialInst->SetScalarParameterValue(FName(BrightnessParamName), BrightnessUsable);
	}
}

void ATRShrine::SetMatToUnusable()
{
	if (GemMaterialInst)
	{
		GemMaterialInst->SetVectorParameterValue(FName(BaseColorParamName), BaseColorUnusable);
		GemMaterialInst->SetVectorParameterValue(FName(LightColorParamName), LightColorUnusable);
		GemMaterialInst->SetScalarParameterValue(FName(BrightnessParamName), BrightnessUnusable);
	}
}

void ATRShrine::SetLightToUsable()
{
	if (GemLightComp)
	{
		GemLightComp->SetLightColor(LightColorUsable);
		GemLightComp->SetIntensity(LightIntensityUsable);
	}
}

void ATRShrine::SetLightToUnusable()
{
	if (GemLightComp)
	{
		GemLightComp->SetVisibility(false);
	}
}
