// Copyright (C) 2024 by Haguk Kim


#include "TRUtils.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "BotCharacter.h"
#include "NavigationSystem.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UNiagaraComponent* TRUtils::TRSpawnSystemAtLocation(TQueue<TWeakObjectPtr<UNiagaraComponent>>& Pool, int32& CurrPoolCnt, int32 MaxCoexistCnt, const UWorld* World, UNiagaraSystem* SystemTemplate, FVector SpawnLocation, FRotator SpawnRotation, FVector Scale, bool bAutoActivate, bool bPreCullCheck)
{
    UNiagaraComponent* Spawned = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        SystemTemplate,
        SpawnLocation,
        SpawnRotation,
        Scale,
        true,
        bAutoActivate,
        ENCPoolMethod::ManualRelease,
        bPreCullCheck
    );
    Pool.Enqueue(MakeWeakObjectPtr<UNiagaraComponent>(Spawned));
    CurrPoolCnt++;

    TRUtils::TRAutoReleaseNiagaraPool(
        Pool,
        CurrPoolCnt,
        MaxCoexistCnt
    );
    return Spawned;
}

void TRUtils::TRAutoReleaseNiagaraPool(TQueue<TWeakObjectPtr<UNiagaraComponent>>& Pool, int32& CurrPoolCnt, int32 MaxCoexistCnt)
{
    // NOTE: 
    // 언리얼의 기본 AutoRelease 설정과 다르게 Free 항목을 실시간으로 틱마다 유지시켜주는 방식이 아니며,
    // TRAutoReleaseNiagaraPool를 호출할 때에만 상태가 갱신된다
    // 따라서 이미 재생이 완료된 인스턴스가 Free상태가 아닐 수 있다

    // 재생 완료된 항목들을 우선적으로 풀에 반환한다
    TQueue<TWeakObjectPtr<UNiagaraComponent>> FilteredQueue;
    TWeakObjectPtr<UNiagaraComponent> Element;
    while (Pool.Dequeue(Element))
    {
        if (!Element.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("TRAutoReleaseNiagaraPool - WeakPtr is invalid! Possibility of memory leak!"));
            continue;
        }

        if (Element->IsActive())
        {
            FilteredQueue.Enqueue(Element);
        }
        else
        {
            Element->ReleaseToPool();
            Element.Get();
        }
    }
    CurrPoolCnt = 0;
    while (FilteredQueue.Dequeue(Element))
    {
        Pool.Enqueue(Element);
        CurrPoolCnt++;
    }

    // 그럼에도 풀 크기가 최대 크기를 초과하는 경우, 재생중인 인스턴스를 강제로 풀로 반환한다
    int32 ReleaseCnt = CurrPoolCnt - MaxCoexistCnt; // 음수일 수 있음
    while (ReleaseCnt > 0 && !Pool.IsEmpty())
    {
        ReleaseCnt--;

        TWeakObjectPtr<UNiagaraComponent> ReleasePending = nullptr;
        Pool.Dequeue(ReleasePending);
        CurrPoolCnt--;
        if (ReleasePending.IsValid())
        {
            // 중요: 즉각적인 디액티베이션이 필수적임
            // 일반 디액티베이션을 호출할 경우 풀에 반환되더라도 사용 가능하게 될때까지 1틱이 소요됨
            // 이는 아직 종료되지 않은 나이아가라 컴포넌트를 ReleaseToPool할 경우
            // 풀 내부에서 ManualRelease_Complete 상태에 먼저 진입하고 그 다음 틱에 ManualRelease가 처리되기 때문임
            // NOTE: 만약 예기치 못한 문제가 추후 발생한다면 FNCPool::Reclaim 내 구현부를 참고해 필요한 작업을 추가해볼 것
            UNiagaraComponent* Comp = ReleasePending.Get();
            Comp->DeactivateImmediate();
            Comp->ReleaseToPool();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("TRAutoReleaseNiagaraPool - Unexpected invalid weak ref!"));
        }
    }
}

FString TRUtils::TimeSecondsToString(float InSeconds)
{
    const TCHAR* NegativeModifier = InSeconds < 0.f ? TEXT("-") : TEXT("");
    InSeconds = FMath::Abs(InSeconds);
    const int32 NumMinutes = FMath::FloorToInt(InSeconds / 60.f);
    const int32 NumSeconds = FMath::FloorToInt(InSeconds - (NumMinutes * 60.f));
    return FString::Printf(TEXT("%s%02d:%02d"), NegativeModifier, NumMinutes, NumSeconds);
}

float TRUtils::GetFallOffMultOfDist(float Distance, float MaxDistDmgMult, float MaxFallOffDist, float MinFallOffDist)
{
    if (Distance <= MinFallOffDist) return 1.0f;
    float DistAlpha = FMath::Min(Distance, MaxFallOffDist);
    return FMath::Lerp(1.0f, MaxDistDmgMult, (DistAlpha - MinFallOffDist) / (MaxFallOffDist - MinFallOffDist));
}

const TArray<FDropItem> TRUtils::SelectDropItems(const TArray<FDropItem>& Candidates)
{
	TArray<FDropItem> Result;
	for (const FDropItem& Loot : Candidates)
	{
		float DropChance = FMath::Clamp(Loot.DropRate, 0, 1);
		if (FMath::FRand() <= Loot.DropRate)
		{
			Result.Add(Loot);
		}
	}
	return Result;
}

const TArray<FDropItem> TRUtils::FilterAndSelectCandidates(const TArray<FUnfilteredDropItem>& Candidates, int32 DungeonDepth, int32 Count)
{
    TArray<const FDropItem*> Selected;
    TArray<float> Weights;
    for (const FUnfilteredDropItem& Cand : Candidates)
    {
        if (Cand.MaxDepth >= DungeonDepth && DungeonDepth >= Cand.MinDepth)
        {
            Selected.Add(&Cand.DropItem);
            Weights.Add(Cand.SelectWeight);
        }
    }

    TArray<FDropItem> Result;
    for (int32 i = 0; i < Count; ++i)
    {
        Result.Add(*TRUtils::GetRandomElementByWeight(Selected, Weights));
    }
    return Result;
}

TSubclassOf<ABotCharacter> TRUtils::SelectSpawnMonster(TArray<FSpawnableMonsterData>& Candidates)
{
    if (!Candidates.IsEmpty())
    {
        TArray<TSubclassOf<ABotCharacter>*> Elements;
        TArray<float> Weights;
        for (int32 Idx = 0; Idx < Candidates.Num(); ++Idx)
        {
            Elements.Add(&Candidates[Idx].BotClass);
            Weights.Add(Candidates[Idx].SpawnRate);
        }
        TSubclassOf<ABotCharacter>* Chosen = GetRandomElementByWeight<TSubclassOf<ABotCharacter>>(Elements, Weights);
        return *Chosen;
    }
    return nullptr;
}

float TRUtils::FindLargestElement(const FVector& Vector)
{
    return FMath::Max(FMath::Max(Vector.X, Vector.Y), Vector.Z);
}

FVector TRUtils::FindRandomNavigationPoint(UWorld* World, const FVector& Center, float Radius)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("FindRandomNavigationPoint - Invalid world!"));
        return FVector::ZeroVector;
    }

    FVector FoundLocation = Center;
    FNavLocation RandomPoint;
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (NavSys && NavSys->GetRandomPointInNavigableRadius(Center, Radius, RandomPoint))
    {
        FoundLocation = RandomPoint.Location;
    }
    return FoundLocation;
}

bool TRUtils::GetMovablePointNearActor(UWorld* World, AActor* Target, FName CollisionProfileName, float VertDistLimit, float HorDistLimit, FVector& out_Point)
{
    if (!World || !Target) return false;

    // XY좌표 기준으로 바닥을 찾아 네브포인트를 찾는다
    FHitResult GroundHit;
    FVector TargetLocation = Target->GetActorLocation();
    FVector LineEnd = TargetLocation;
    LineEnd.Z -= VertDistLimit;

    FCollisionQueryParams GroundColParams;
    GroundColParams.AddIgnoredActor(Target); // 타깃은 무시

    World->LineTraceSingleByProfile(GroundHit, TargetLocation, LineEnd, CollisionProfileName, GroundColParams);
    out_Point = TRUtils::FindRandomNavigationPoint(World, Target->GetActorLocation(), HorDistLimit);

    return true;
}

bool TRUtils::IsAllyWith(AGameCharacter* CharA, AGameCharacter* CharB)
{
    if (!CharA || !CharB) return false;
    if (CharA == CharB) return true;
    if (CharA->IsA<AFPSCharacter>() && CharB->IsA<AFPSCharacter>())
    {
        return true;
    }
    else if (CharA->IsA<ABotCharacter>() && CharB->IsA<ABotCharacter>())
    {
        return true;
    }
    return false;
}

void TRUtils::ApplyStatusEffectsOnTarget(const TArray<FStatEffectGenInfo>& Effects, AGameCharacter* Target, AGameCharacter* Applier)
{
    if (!IsValid(Target))
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyStatusEffectsOnTarget - Invalid target"));
        return;
    }
    for (const FStatEffectGenInfo& Effect : Effects)
    {
        Target->Server_GenerateAndAddStatEffect(Effect, Applier);
    }
}

bool TRUtils::GetRandomSpawnLocation(const TArray<class ASpawnPoint*>& SpawnPoints, ESpawnPointType SearchType, FVector& out_SpawnLocation)
{
    TArray<ASpawnPoint*> ValidCandidates;
    for (ASpawnPoint* SpawnPoint : SpawnPoints)
    {
        if (SpawnPoint && SpawnPoint->SpawnType == SearchType)
        {
            ValidCandidates.Add(SpawnPoint);
        }
    }
    if (!ValidCandidates.IsEmpty())
    {
        ASpawnPoint* ChosenPoint = ValidCandidates[FMath::Rand() % ValidCandidates.Num()];
        if (ChosenPoint)
        {
            out_SpawnLocation = ChosenPoint->GetRandomSpawnLocation();
            return true;
        }
    }
    out_SpawnLocation = FVector::ZeroVector;
    return false;
}

void TRUtils::OptimizePrimitiveComp(UPrimitiveComponent* Component, bool bCastShadows, bool bDisableLights)
{
    if (!Component) return;
    Component->SetCastShadow(bCastShadows);
    if (bDisableLights)
    {
        Component->SetAffectDistanceFieldLighting(false);
        Component->SetAffectDynamicIndirectLighting(false);
        Component->SetLightingChannels(false, false, false);
    }
}
