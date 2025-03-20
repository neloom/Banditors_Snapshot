// Copyright (C) 2025 by Haguk Kim


#include "SoundSubsystem.h"
#include "Kismet/GameplayStatics.h"

void USoundSubsystem::PlayBG(USoundCue* SoundCue, float FadeInTime)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaySoundAtLocation - Invalid SoundCue!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaySoundAtLocation - Invalid World!"));
        return;
    }

    if (!GlobalSoundComp)
    {
        GlobalSoundComp = UGameplayStatics::CreateSound2D(World, SoundCue);
    }

    if (GlobalSoundComp)
    {
        GlobalSoundComp->FadeIn(FadeInTime, 1.0f);
        GlobalSoundComp->Play();
    }
}

void USoundSubsystem::StopBG(float FadeOutTime)
{
    if (GlobalSoundComp)
    {
        GlobalSoundComp->FadeOut(FadeOutTime, 0.0f);
    }
}

void USoundSubsystem::PlaySoundAtLocation(USoundCue* SoundCue, FVector Location)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaySoundAtLocation - Invalid SoundCue!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaySoundAtLocation - Invalid World!"));
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(World, SoundCue, Location);
}

void USoundSubsystem::PlaySound2D(USoundCue* SoundCue)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaySoundAtLocation - Invalid SoundCue!"));
        return;
    }

    UGameplayStatics::PlaySound2D(GetWorld(), SoundCue);
}
