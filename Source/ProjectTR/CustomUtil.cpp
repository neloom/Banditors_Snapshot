// Copyright (C) 2024 by Haguk Kim


#include "CustomUtil.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"

CustomUtil::CustomUtil()
{
}

CustomUtil::~CustomUtil()
{
}

void CustomUtil::SetGraphicsSettingsCustom()
{
    if (GEngine)
    {
        // 콘솔 설정
        // NOTE: 직접 세팅을 수정해 저장하는 행위는 지양할 것
        GEngine->Exec(nullptr, TEXT("r.ViewDistanceScale 0.5"));
        GEngine->Exec(nullptr, TEXT("r.PostProcessAAQuality 1"));
        GEngine->Exec(nullptr, TEXT("r.DetailMode 1"));
        GEngine->Exec(nullptr, TEXT("r.Shadow.Virtual.MaxPhysicalPages 8192"));
        GEngine->Exec(nullptr, TEXT("sg.ShadowQuality 1"));
        GEngine->Exec(nullptr, TEXT("sg.TextureQuality 1"));
        GEngine->Exec(nullptr, TEXT("sg.EffectsQuality 1"));
        GEngine->Exec(nullptr, TEXT("FoliageQuality 1"));
    }
}