// Copyright (C) 2025 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SoundSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API USoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    // 글로벌 사운드 컴포넌트
    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlayBG(USoundCue* SoundCue, float FadeInTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void StopBG(float FadeOutTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlaySoundAtLocation(USoundCue* SoundCue, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlaySound2D(USoundCue* SoundCue);
	
private:
	UPROPERTY()
	UAudioComponent* GlobalSoundComp;
};
