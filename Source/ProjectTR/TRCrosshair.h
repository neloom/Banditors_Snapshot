// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TRWidget.h"
#include "FPSCharacter.h"
#include "TRCrosshair.generated.h"

UENUM(BlueprintType)
enum class ECrosshairDotShapeEnum : uint8
{
	CDS_Circle,
	CDS_Square,
};

USTRUCT(BlueprintType)
struct FCrosshairStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Gap"))
	int32 Gap = 8;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Length"))
	int32 Length = 6;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Thickness"))
	int32 Thickness = 2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Opacity"))
	float Opacity = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Outline Opacity"))
	float OutlineOpacity = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Outline Thickness"))
	int32 OutlineThickness = 2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Dot Shape"))
	ECrosshairDotShapeEnum DotShape = ECrosshairDotShapeEnum::CDS_Circle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Dot Size"))
	int32 DotSize = 2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Dot Opacity"))
	float DotOpacity = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Dot Outline Thickness"))
	int32 DotOutlineThickness = 2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Dot Outline Opacity"))
	float DotOutlineOpacity = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Color"))
	FLinearColor Color = FLinearColor(1,1,1,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Outline Color"))
	FLinearColor OutlineColor = FLinearColor(0,0,0,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Hide Right Line?"))
	bool bHideRightLine = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Hide Left Line?"))
	bool bHideLeftLine = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Hide Top Line?"))
	bool bHideTopLine = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Hide Bottom Line?"))
	bool bHideBottomLine = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Crosshair Render Scale"))
	float RenderScale = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Enable Hitmarker?"))
	bool bEnableHitmarker = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Hitmarker Color"))
	FLinearColor HitmarkerColor = FLinearColor(1,1,1,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Hitmarker Headshot Color"))
	FLinearColor HitmarkerHeadshotColor = FLinearColor(1,0,0,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Hitmarker Kill Color"))
	FLinearColor HitmarkerKillColor = FLinearColor(1,0,0,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Hitmarker Render Scale"))
	float HitmarkerRenderScale = 1.0f;
};

/**
 * 
 */
UCLASS()
class PROJECTTR_API UTRCrosshair : public UTRWidget
{
	GENERATED_BODY()
	
public:
	// 현재 크로스헤어 구성정보
	UPROPERTY(BlueprintReadWrite)
	FCrosshairStruct Settings;

public:
	// 새 정보를 기반으로 크로스헤어를 재생성한다
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RefreshCrosshair(FCrosshairStruct NewSettings);

	// 처치 시, 크리티컬 시에는 기본 히트와 다른 별개의 효과를 재생한다
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PlayHitEffect(bool bIsKillshot, bool bIsCrit);

	// 피격 시 피격 방향에 피격 마커를 생성한다
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DrawHitCircle(FVector DamageLocation, AFPSCharacter* DamagedTarget, float Damage, float Duration = 1.0f);
};
