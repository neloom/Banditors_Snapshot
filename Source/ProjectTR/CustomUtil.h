// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Math/Matrix.h"

/**
 * 
 */
class PROJECTTR_API CustomUtil
{
public:
	CustomUtil();
	~CustomUtil();

    // UObject의 하위타입 T에 대한 TSubclassOf를 인자로 받아 Outer를 가지는 오브젝트를 생성 후 TUniquePtr 반환
    template<typename T>
    static TUniquePtr<T> CreateUniqueObjectFromSubclass(TSubclassOf<T> Subclass, UObject* Outer)
    {
        if (Subclass)
        {
            if (!Subclass.Get()->IsChildOf(UObject::StaticClass()))
            {
                UE_LOG(LogTemp, Error, TEXT("Unable to create TUniquePtr from non-UObject-based object %s"), *(Subclass.Get()->GetName()));
                return nullptr;
            }
            T* ObjectInstance = NewObject<T>(Outer, Subclass.Get());
            return TUniquePtr<T>(ObjectInstance);
        }
        return nullptr;
    }

    // UObject의 하위타입 T에 대한 TSubclassOf를 인자로 받아 Outer를 가지는 오브젝트를 생성 후 TSharedPtr 반환
    template<typename T>
    static TSharedPtr<T> CreateSharedObjectFromSubclass(TSubclassOf<T> Subclass, UObject* Outer)
    {
        if (Subclass)
        {
            if (!Subclass.Get()->IsChildOf(UObject::StaticClass()))
            {
                UE_LOG(LogTemp, Error, TEXT("Unable to create TSharedPtr from non-UObject-based object %s"), *(Subclass.Get()->GetName()));
                return nullptr;
            }
            T* ObjectInstance = NewObject<T>(Outer, Subclass.Get());
            return TSharedPtr<T>(ObjectInstance);
        }
        return nullptr;
    }

    // 그래픽 설정을 낮게 변경한다
    static void SetGraphicsSettingsCustom();
};
