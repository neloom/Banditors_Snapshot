// Copyright (C) 2024 by Haguk Kim


#include "TRExplosion.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "TRPlayerController.h"
#include "TRUtils.h"
#include "TRDamageType.h"


ATRExplosion::ATRExplosion()
{
	PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    ExplRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("ExplRoot"));
    check(ExplRootComp != nullptr);
    SetRootComponent(ExplRootComp);

    // 기본 VFX 컨픽 바인딩
    if (!FxConfig)
    {
        static ConstructorHelpers::FObjectFinder<UFxConfig> FXFinder(TEXT(ASSET_DEFAULT_FX));
        if (FXFinder.Succeeded())
        {
            FxConfig = FXFinder.Object;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ATRExplosion - Unable to find default FX config asset!"));
        }
    }
}

void ATRExplosion::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (HasAuthority())
    {
        if (FxConfig)
        {
            ExplosionVFX = FxConfig->SearchNiagaraFromEnum(ExplosionInfo.ExplosionVFXEnum);
        }
        float ScaleScalar = (ExplosionInfo.ExplosionRadius * ExplosionInfo.VFXRadiusConstant);
        ExplosionVFXScale = FVector(ScaleScalar, ScaleScalar, ScaleScalar);
    }
}

void ATRExplosion::BeginPlay()
{
	Super::BeginPlay();
	
    if (HasAuthority() && ExplosionInfo.bExplodeOnBeginPlay)
    {
        Server_Explode();
    }
}

void ATRExplosion::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATRExplosion::Server_Explode()
{
    Multicast_Explode();

    TArray<AActor*> Targets;
    TArray<AActor*> Ignored = { this };

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectQuery;
    for (const TEnumAsByte<ECollisionChannel>& ECC : ExplosionInfo.ExplosionTargetType)
    {
        TEnumAsByte<EObjectTypeQuery> QueryType = UEngineTypes::ConvertToObjectType(ECC);
        ObjectQuery.Add(QueryType);
    }

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        this->GetActorLocation(),
        ExplosionInfo.ExplosionRadius,
        ObjectQuery,
        nullptr,
        Ignored,
        Targets
    );

    for (AActor* Target : Targets)
    {
        UPrimitiveComponent* PhysComp = nullptr;

        AGameCharacter* GameCharacter = Cast<AGameCharacter>(Target);
        ABaseItem* GameItem = Cast<ABaseItem>(Target);

        // 폭발 지점과 대상 사이에 장애물이 없는지 확인
        FVector BlockCheckStartLoc = GetActorLocation() + (CollisionNormal.GetSafeNormal() * BlockCheckDeltaSize);
        if (Target && !IsBlocked(BlockCheckStartLoc, Target->GetActorLocation()))
        {
            if (GameCharacter)
            {
                if (ExplosionInfo.bApplyDamageOnExplosion)
                {
                    FHitResult ExplosionHitRes;
                    FVector TargetLoc = GameCharacter->GetActorLocation();
                    FVector ExplosionLoc = this->GetActorLocation();
                    ExplosionHitRes.ImpactNormal = ExplosionLoc - TargetLoc;
                    ExplosionHitRes.ImpactPoint = TargetLoc;
                    ExplosionHitRes.TraceStart = ExplosionLoc;
                    ExplosionHitRes.Location = TargetLoc;
                    // 필요 시 나머지 정보 기입

                    UGameplayStatics::ApplyPointDamage(
                        GameCharacter,
                        GetDamageOnTarget(GameCharacter),
                        GameCharacter->GetActorLocation() - ExplosionHitRes.TraceStart,
                        ExplosionHitRes,
                        GetInstigatorController(),
                        GetInstigator(),
                        ExplosionInfo.ExplosionDamageType
                    );
                }
                PhysComp = Cast<UPrimitiveComponent>(GameCharacter->GetCapsuleComponent());
                // TODO: 래그돌
#if WITH_EDITOR
                //DrawDebugLine(GetWorld(), this->GetActorLocation(), GameCharacter->GetActorLocation(), FColor::Blue, false, 10.0f);
#endif
            }
            else if (GameItem)
            {
                PhysComp = GameItem->GetPhysComponent();
                // TODO: 필요 시 로직 추가
#if WITH_EDITOR
                //DrawDebugLine(GetWorld(), this->GetActorLocation(), GameItem->GetActorLocation(), FColor::Green, false, 10.0f);
#endif
            }
            else
            {
                // TODO: 현재로는 캐릭터와 아이템에만 폭발 로직을 적용
            }

            // 공통 물리 로직
            // 적용 대상에게만 사용
            if (PhysComp && ExplosionInfo.bApplyImpactOnExplosion && ExplosionInfo.ExplosionTargetType.Contains(PhysComp->GetCollisionObjectType()))
            {
                PhysComp->AddRadialImpulse(GetActorLocation(), ExplosionInfo.ExplosionRadius, ExplosionInfo.BaseImpactStrength, static_cast<ERadialImpulseFalloff>(ExplosionInfo.ImpactFalloffType), true /* TODO: 테스트 필요 */);
            }
        }
    }

#if WITH_EDITOR
    //DrawDebugSphere(GetWorld(), this->GetActorLocation(), ExplosionInfo.ExplosionRadius, 8, FColor::Yellow, false, 10.0f);
#endif

    if (ExplosionInfo.bDestroyAfterExplosion)
    {
        Destroy();
    }
}

void ATRExplosion::Multicast_Explode_Implementation()
{
    Local_PlayExplosionVFX();
}

float ATRExplosion::GetDamageOnTarget(class AGameCharacter* Target)
{
    if (!Target || ExplosionInfo.ExplosionRadius <= 0) return 0;
    float Damage = FMath::Lerp(
        ExplosionInfo.BaseDamage, // 최대 데미지 (point blank)
        ExplosionInfo.MinExplosionMultiplier * ExplosionInfo.BaseDamage, // 최소 데미지
        FMath::Min(1.0f, GetDistanceTo(Target) / ExplosionInfo.ExplosionRadius) // 최소 데미지보다 적은 데미지를 받을 수 없다
    );
    
    APawn* ExplInstigator = GetInstigator();
    if (ExplInstigator && (Target == ExplInstigator || (Target->IsA<AFPSCharacter>() && ExplInstigator->IsA<AFPSCharacter>())))
    {
        Damage *= ExplosionInfo.DmgMultOnExplInstigator;
    }
    return Damage;
}

bool ATRExplosion::IsBlocked(FVector StartLocation, FVector TargetLocation)
{
    FCollisionQueryParams TraceParams(FName(TEXT("ExplosionLinetrace")), false, this);
    TraceParams.bReturnPhysicalMaterial = false;
    TraceParams.AddIgnoredActor(this);
    FCollisionResponseParams ColParams;

    // 블로킹 대상 채널 설정
    ColParams.CollisionResponse.SetAllChannels(ECR_Ignore);
    for (const ECollisionChannel& BlockingChannel : ExplosionInfo.ExplosionBlockedByType)
    {
        ColParams.CollisionResponse.SetResponse(BlockingChannel, ECR_Block);
    }

    FHitResult HitRes;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitRes,
        StartLocation,
        TargetLocation,
        ECC_Explosion,
        TraceParams,
        ColParams
    );

    if (bHit)
    {
#if WITH_EDITOR
        //DrawDebugLine(GetWorld(), StartLocation, TargetLocation, FColor::Red, false, 10.0f);
        //DrawDebugPoint(GetWorld(), HitRes.Location, 10.0f, FColor::Yellow, false, 10.0f);
#endif
        return true;
    }
    return false;
}

void ATRExplosion::Local_PlayExplosionVFX()
{
    ATRPlayerController* TRPC = GetWorld()->GetFirstPlayerController<ATRPlayerController>();
    if (!TRPC || !ExplosionVFX) return;
    TRUtils::TRSpawnSystemAtLocation(
        TRPC->ExplVFXPendingRelease,
        TRPC->CurrExplVFXPendingReleaseCnt,
        TRPC->MaxCoexistingExplVFXCnt,
        GetWorld(),
        ExplosionVFX,
        GetActorLocation(),
        GetActorRotation(),
        ExplosionVFXScale,
        true,
        true
    );
}

void ATRExplosion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

