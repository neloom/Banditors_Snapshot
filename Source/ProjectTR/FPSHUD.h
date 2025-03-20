// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "TRCrosshair.h"
#include "FPSHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API AFPSHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

public:
	// 사용할 크로스헤어 클래스
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UTRCrosshair> CrosshairClass = nullptr;

	// 현재 사용중인 크로스헤어 위젯 인스턴스
	UPROPERTY(BlueprintReadWrite)
	class UTRCrosshair* Crosshair = nullptr;
};
