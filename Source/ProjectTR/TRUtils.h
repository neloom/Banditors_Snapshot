// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRStructs.h"
#include "StatusEffect.h"
#include "SpawnPoint.h"
#include "Kismet/KismetMathLibrary.h"

/**
 * 
 */
class PROJECTTR_API TRUtils
{
public:
    // Weight 기반 랜덤한 값을 선택하기 위해 사용할 수 있는 함수
    template<typename T>
    static T* GetRandomElementByWeight(const TArray<T*>& Elements, const TArray<float>& Weights)
    {
        // 배열의 크기가 같아야 함
        if (Elements.Num() != Weights.Num() || Elements.Num() == 0)
        {
            return nullptr; // 에러 발생 시 nullptr 반환
        }

        // weight 합 계산
        float TotalWeight = 0.0f;
        for (float Weight : Weights)
        {
            TotalWeight += Weight;
        }

        // 무게 합 이내의 랜덤한 실수 선택
        float RandomWeight = UKismetMathLibrary::RandomFloatInRange(0.0f, TotalWeight);

        // 선택한 실수 범위에 해당하는 원소 반환
        float CumulativeWeight = 0.0f;
        for (int32 i = 0; i < Weights.Num(); ++i)
        {
            CumulativeWeight += Weights[i];
            if (RandomWeight <= CumulativeWeight)
            {
                return Elements[i];
            }
        }
        return Elements.Last(); // floating-point precision issue 발생 시 마지막 값 반환
    }

    template<typename T>
    static void ShuffleArray(TArray<T>& Array)
    {
        for (int32 i = Array.Num() - 1; i > 0; --i)
        {
            int32 RandomIndex = FMath::RandRange(0, i);
            Array.Swap(i, RandomIndex);
        }
    }

    // 커스텀 나이아가라 풀링 관리용 함수
    static class UNiagaraComponent* TRSpawnSystemAtLocation(TQueue<TWeakObjectPtr<class UNiagaraComponent>>& Pool, int32& CurrPoolCnt, int32 MaxCoexistCnt, const UWorld* World, UNiagaraSystem* SystemTemplate, FVector SpawnLocation, FRotator SpawnRotation, FVector Scale, bool bAutoActivate, bool bPreCullCheck);
    static void TRAutoReleaseNiagaraPool(TQueue<TWeakObjectPtr<class UNiagaraComponent>>& Pool, int32& CurrPoolCnt, int32 MaxCoexistCnt);

    // UKismetStringLibrary의 동명의 함수의 밀리초를 제거한 버전
    static FString TimeSecondsToString(float InSeconds);

    // 주어진 거리에 대한 데미지 증감률을 반환한다
    // 거리가 MinFallOffDist 이하면 1을, MaxFallOffDist면 MaxDistDmgMult를, 그 사이의 경우 Lerp한 값을 반환한다
    static float GetFallOffMultOfDist(float Distance, float MaxDistDmgMult, float MaxFallOffDist, float MinFallOffDist);

    // 주어진 드랍 가능 후보들을 랜덤으로 선택해 결정된 항목들을 반환한다
    static const TArray<FDropItem> SelectDropItems(const TArray<FDropItem>& Candidates);

    // 주어진 조건에 맞는 드랍(혹은 상점 판매) 가능 후보들만 전부 반환한다
    static const TArray<FDropItem> FilterAndSelectCandidates(const TArray<FUnfilteredDropItem>& Candidates, int32 DungeonDepth, int32 Count);

    // 주어진 몬스터 정보들에 대해 각각의 확률을 기반으로 랜덤하게 항목을 선택해 반환한다
    // 특수한 상황에 의한 스폰 확률 조정이 필요할 경우 이 함수를 호출하기 전에 사전에 데이터 값을 수정하는 것으로 처리해 주어야 한다
    static TSubclassOf<class ABotCharacter> SelectSpawnMonster(TArray<FSpawnableMonsterData>& Candidates);

    // 벡터 요소 중 가장 큰 값을 찾아 반환한다
    static float FindLargestElement(const FVector& Vector);

    // 주어진 위치를 기준으로 주변 반경 중 네비메쉬가 존재하는 영역의 임의의 공간 한 곳을 찾아 반환한다
    static FVector FindRandomNavigationPoint(UWorld* World, const FVector& Center, float Radius);

    // 인자로 주어진 액터 인근의 걸을 수 있는 지점을 반환한다
    // 가급적 타깃 X,Y와 동일하거나 인접한 지점을 찾으며, 만약 찾은 지점의 거리가 DistanceLimit을 초과하면 false를 반환한다
    static bool GetMovablePointNearActor(UWorld* World, AActor* Target, FName CollisionProfileName, float VertDistLimit, float HorDistLimit, FVector& out_Point);

    // 아군인지 여부를 반환한다
    static bool IsAllyWith(class AGameCharacter* CharA, class AGameCharacter* CharB);

    // 상태이상 효과를 적용한다
    static void ApplyStatusEffectsOnTarget(const TArray<FStatEffectGenInfo>& Effects, class AGameCharacter* Target, class AGameCharacter* Applier);

    // 스폰포인트의 후보가 주어지면 주어진 타입에 맞는 후보들을 추린 후 그 중 랜덤으로 포인트를 선택하고, 
    // 그 포인트의 조건에 맞게 랜덤한 위치를 선택해 반환한다
    // 만약 조건에 맞는 포인트가 존재하지 않는 경우 false를, 발견했다면 true를 반환한다
    static bool GetRandomSpawnLocation(const TArray<class ASpawnPoint*>& SpawnPoints, ESpawnPointType SearchType, FVector& out_SpawnLocation);

    // 주어진 PrimitiveComp의 라이팅, 섀도우 등을 조절하여 최적화하기 위해 사용할 수 있다
    static void OptimizePrimitiveComp(class UPrimitiveComponent* Component, bool bCastShadows = false, bool bDisableLights = true);
};
