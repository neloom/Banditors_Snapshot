// Copyright (C) 2024 by Haguk Kim


#include "BaseProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "TRDamageType.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "TRProjMovementComponent.h"
#include "HitboxComponent.h"
#include "OuterHitboxComponent.h"
#include "GameCharacter.h"
#include "ProjectTRGameModeBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "FxConfig.h"
#include "GunItem.h"
#include "FPSCharacter.h"
#include "TRMacros.h"
#include "TRUtils.h"
#include "DamageTypeNeutral.h"

// Sets default values
ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

    InitialLifeSpan = 30.0f;

#pragma region /** Networking */
    bReplicates = true;
    SetReplicateMovement(true);
    NetCullDistanceSquared = MAX_FLT;
    NetUpdateFrequency = TR_PROJ_MOVEREPL_RATE;
#pragma endregion

#pragma region /** Component Initialization */
    // ProjectileMeshComponent
    if (!ProjectileMeshComponent)
    {
        ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjMeshComponent"));

        // 메쉬 기본값
        static ConstructorHelpers::FObjectFinder<UStaticMesh>Mesh(TEXT("/Game/DefaultProjectileMesh.DefaultProjectileMesh"));
        if (Mesh.Succeeded())
        {
            ProjectileMeshComponent->SetStaticMesh(Mesh.Object);
            ProjectileMeshComponent->SetRelativeScale3D(FVector(0.09f, 0.09f, 0.09f));
        }
    }
    InitProjectileMesh(ProjectileMeshComponent);
    RootComponent = ProjectileMeshComponent;
    ProjectileMeshComponent->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);

    if (!ProjectileMovementComponent)
    {
        ProjectileMovementComponent = CreateDefaultSubobject<UTRProjMovementComponent>(TEXT("TRProjMoveComponent"));
        ProjectileMovementComponent->SetUpdatedComponent(ProjectileMeshComponent);
        InitProjectileMovement(ProjectileMovementComponent);
    }
#pragma endregion

    if (!DamageType)
    {
        // 기본값 Neutral
        DamageType = UDamageTypeNeutral::StaticClass();
    }

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
            UE_LOG(LogTemp, Error, TEXT("ABaseProjectile - Unable to find default FX config asset!"));
        }
    }
}

FRotator ABaseProjectile::GetHitRotationForVFX(const FHitResult& Hit) const
{
    FVector VFXRotationVector = (bUseHitNormalForVFX) ? Hit.Normal : (this->GetActorForwardVector() * -1);
    return VFXRotationVector.Rotation();
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
    // VFX 초기화
    Local_InitializeVFX();

    // 자기 자신 및 격발자와의 콜리전을 완전히 무시
    ProjectileMeshComponent->IgnoreActorWhenMoving(this, true);
    ProjectileMeshComponent->IgnoreActorWhenMoving(GetInstigator(), true);

    InitialLocation = GetActorLocation();

    // 클라이언트 로직
    if (!HasAuthority())
    {
        // 충돌 연산 해제
        ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // 무브먼트 연산 해제
        ProjectileMovementComponent->Deactivate();
    }
}

void ABaseProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 파괴 전 VFX 제거
    Local_CleanUpVFX();
    Super::EndPlay(EndPlayReason);
}

void ABaseProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority())
    {
        SetActorLocationAndRotation(
            FMath::VInterpTo(GetActorLocation(), Client_ProjLerpTargetLocation, DeltaTime, TR_PROJ_MOVEREPL_RATE),
            FMath::RInterpTo(GetActorRotation(), Client_ProjLerpTargetRotation, DeltaTime, TR_PROJ_MOVEREPL_RATE),
            /*bSweep=*/ false
        );
    }
}

void ABaseProjectile::PostNetReceiveLocationAndRotation()
{
    // Super::PostNetReceiveLocationAndRotation 참고
    const FRepMovement& LocalRepMovement = GetReplicatedMovement();
    FVector NewLocation = FRepMovement::RebaseOntoLocalOrigin(LocalRepMovement.Location, this);

    if (RootComponent && RootComponent->IsRegistered() && (NewLocation != GetActorLocation() || LocalRepMovement.Rotation != GetActorRotation()))
    {
        if (!HasAuthority())
        {
            Client_ProjLerpTargetLocation = NewLocation;
            Client_ProjLerpTargetRotation = LocalRepMovement.Rotation;
        }
    }
}

void ABaseProjectile::Server_DestoryProjectile()
{
    Destroy();
}

void ABaseProjectile::InitProjectileMovement(UTRProjMovementComponent* Component)
{
    check(Component != nullptr);
    Component->bUseDefaultHitHandlers = false; // 커스텀 핸들러를 사용해 관통 등의 로직을 처리

    Component->InitialSpeed = 3000.0f;
    Component->MaxSpeed = 3000.0f;
    Component->bRotationFollowsVelocity = true;
    Component->bShouldBounce = false; // NOTE: 바운스를 사용하게 될 경우 히트판정 파이프라인 자체가 달라지므로 가급적 해제하는 것이 권장됨
    Component->Bounciness = 0.0f;
    Component->ProjectileGravityScale = 0.0f;
}

void ABaseProjectile::InitProjectileMesh(UStaticMeshComponent* Component)
{
    check(Component != nullptr);
    Component->SetCollisionProfileName(TEXT("Projectile"));
    Component->SetGenerateOverlapEvents(false); // 불필요
    Component->SetSimulatePhysics(false); // 피직스는 무브먼트에서 처리
    Component->SetEnableGravity(false);

    // 최적화
    TRUtils::OptimizePrimitiveComp(Component);

    FWalkableSlopeOverride Walkable;
    Walkable.WalkableSlopeBehavior = EWalkableSlopeBehavior::WalkableSlope_Unwalkable;
    Component->SetWalkableSlopeOverride(Walkable);
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority()) return;
    if (!ProjectileMovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("ABaseProjectile::OnHit - Invalid ProjMoveComp!"));
        return;
    }

    // NOTE: IgnoreActorWhenMoving를 등록했기 때문에 애초에 자기 자신 및 Instigator와 Hit가 발생해서는 안됨; 예방적 코드
    if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
    {
        FRotator VFXRotator = GetHitRotationForVFX(Hit);
        bool bIsEffectiveHit = true;

        // 캐릭터 충돌
        AGameCharacter* HitChar = Cast<AGameCharacter>(OtherActor);
        if (HitChar)
        {
            UOuterHitboxComponent* OuterHitboxComp = Cast<UOuterHitboxComponent>(OtherComponent);
            if (OuterHitboxComp)
            {
                OuterHitboxComp->OnOuterHitboxCollision(0.0f/*1틱*/);

                UHitboxComponent* HitboxComp = nullptr;

                if (ProjectileMovementComponent->bShouldBounce)
                {
                    // 바운스되는 투사체의 경우, 예외적으로 아우터 히트박스에만 맞아도 적중한 것으로 취급한다
                    // 이는 바운스의 경우 투사체 처리 파이프라인이 달라지기 때문인데, 
                    // 구조적으로 아우터 히트박스는 관통하면서 세부 히트박스는 바운스하도록 만들 수 없다
                    // 관해서는 TRProjMovementComponent.h 참고
                    // 따라서 이 경우 보다 부정확한 방식(거리순)으로 어떤 세부히트박스에 히트했는지를 추정한다
                    HitboxComp = HitChar->GetNearestDetailedHitboxFrom(Hit.Location);
                }
                else
                {
                    // 투사체 궤적 기반 정밀 히트박스 판정 예측
                    // NOTE: 실제 히트박스와의 히트 연산을 처리하기 위해서는 히트박스들을 1틱 이상 유지시켜야 하는데,
                    // 이 연산의 부하가 크기 때문에 다소 부정확하더라도 이 방법을 사용한다
                    // 등속 직선 이동을 하지 않는 투사체의 경우 다소 판정이 부정확 할 수 있다
                    // NOTE: B를 조준했는데 그 앞에 서있던 A의 아우터 히트박스에 맞은 경우, 여기서는 A의 내부히트박스를 계산하기 때문에 A가 '대신 맞는' 일은 벌어지지 않는다
                    // 그 후 A는 Ignore되고, 짧은 시간 후 B의 아우터에 충돌하는 순간 B의 내부 히트박스를 또 다시 계산하며 정상적으로 히트가 처리된다
                    // 히트스캔의 경우 이를 재귀적으로 구현했으나, 투사체의 경우 시간의 흐름에 따라 순서대로 히트가 발생하기 때문에 비교적 깔끔하게 구현이 가능하다

                    float HitCharMaxBoundSize = HitChar->GetComponentsBoundingBox().GetExtent().GetMax();
                    float ProjRadius = this->GetComponentsBoundingBox().GetExtent().GetMax() / 2;

                    ProjectileMovementComponent->PredictProjHitUsingSphereTrace(
                        HitCharMaxBoundSize,
                        ProjRadius,
                        HitComponent->MoveIgnoreActors,/*NOTE: 아직 히트캐릭터는 포함되지 않음*/
                        HitChar,
                        HitboxComp
                    );
                }
                
                if (HitboxComp)
                {
                    OnProjHitboxCollision(HitComponent, HitChar, HitboxComp, NormalImpulse, Hit);

                    // VFX
                    Server_SpawnProjHitCharacterVFX(Hit.Location, VFXRotator);
                }
                else
                {
                    // 히트박스 충돌이 없었을 경우 이 히트이벤트는 게임 로직 상 영향을 주지 않음
                    bIsEffectiveHit = false;
                }
            }
        }
        else
        {
            // 물리 충돌
            if (bShouldAffectForceOnCollision && OtherComponent->IsSimulatingPhysics())
            {
                OtherComponent->AddImpulseAtLocation(ProjectileMovementComponent->Velocity * ImpactMass, Hit.ImpactPoint);
            }

            // VFX
            if (bShouldPlayObjColVFX)
            {
                Server_SpawnProjHitObjectVFX(Hit.Location, VFXRotator);
            }
        }

        // 유효한 충돌에 한해서 처리할 추가 로직들
        if (bIsEffectiveHit)
        {
            // 충돌 시 폭발 생성
            // NOTE: 액터는 물리 연산을 처리하지 않아야 한다
            // 콜리전 핸들링 메소드를 AlwaysSpawn으로 지정하지 않을 경우 크래시가 발생하기 때문; (물리 액터는 충돌 지점에 끼일 수 있음)
            UWorld* World = GetWorld();
            if (bShouldSpawnExplOnCollision && HasAuthority() && World)
            {
                AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
                if (TRGM)
                {
                    FActorSpawnParameters SpawnParam;
                    SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    SpawnParam.Instigator = GetInstigator();
                    ATRExplosion* SpawnedExpl = TRGM->SpawnExplosion(
                        SpawnExplosiveClass, 
                        World, 
                        Hit.ImpactPoint, 
                        Hit.ImpactNormal.Rotation(), 
                        Hit.Normal, 
                        SpawnParam, 
                        NewExplosiveInfo, 
                        bShouldOverrideExplosiveinfo
                    );
                }
            }

            if ((HitChar && bPiercePawns) || (!HitChar && ProjectileMovementComponent->bShouldBounce && bBounceOffObjIndefinitely))
            {
                // 폰 관통, 혹은 영구 바운스 허용의 경우 히트카운트를 세지 않는다
            }
            else
            {
                CurrHitCount++;
            }

            // 격발자 히트 로직
            // NOTE: 투사체의 경우 히트스캔과 달리 미스 로직은 없음
            APawn* ProjInstigator = GetInstigator();
            if (!FMath::IsNearlyZero(SelfDmgOnCharacterHit) && ProjInstigator)
            {
                UGameplayStatics::ApplyDamage(
                    ProjInstigator,
                    SelfDmgOnCharacterHit,
                    ProjInstigator->GetController(),
                    ProjInstigator,
                    UDamageTypeNeutral::StaticClass() // 자가 데미지의 경우 투사체 데미지 타입과 무관
                );
            }
        }

        // 같은 캐릭터에 대해 1회 이상의 히트 이벤트 발생을 방지한다
        // 주의: 캐릭터가 아닌 액터에 대해서는 바운스 등으로 인해 여러 번 히트가 발생할 수 있음
        if (HitChar)
        {
            HitComponent->IgnoreActorWhenMoving(HitChar, true);
        }
        else
        {
            // 오브젝트 충돌의 경우 최초 1회 충돌만 VFX 재생
            // 이는 바운스 등으로 인해 순간적으로 과도한 VFX 요청이 발생하는 일을 막기 위함
            // NOTE: 캐릭터의 경우 이 로직이 필요하지 않음
            bShouldPlayObjColVFX = false;
        }
    }

    if (DestroyOnHitCount >= 1 && DestroyOnHitCount <= CurrHitCount)
    {
        Server_DestoryProjectile();
    }
}

void ABaseProjectile::OnProjHitboxCollision(UPrimitiveComponent* HitComponent, AGameCharacter* HitboxOwner, UHitboxComponent* HitboxComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HitboxComp)
    {
        UE_LOG(LogTemp, Error, TEXT("OnProjHitboxCollision should not be called upon non-hitbox component collision!"));
        return;
    }

    // 상태이상 적용
    AGameCharacter* Shooter = Cast<AGameCharacter>(GetInstigator()); // null일 수 있다; 허용됨
    const TArray<FStatEffectGenInfo>& StatEffectsOnHit = TRUtils::IsAllyWith(Shooter, HitboxOwner) ? StatEffsToAllyWhenHit : StatEffsToEnemyWhenHit;
    TRUtils::ApplyStatusEffectsOnTarget(StatEffectsOnHit, HitboxOwner, Shooter);

    // 데미지 부여
    float HitboxDmgMultiplier = HitboxComp->GetDamageMultiplier();

    float Damage = 0.0f;
    if (HitboxOwner)
    {
        if (TRUtils::IsAllyWith(HitboxOwner, Cast<AGameCharacter>(GetInstigator())))
        {
            Damage = HitboxDmgMultiplier * AllyDamage;
        }
        else
        {
            Damage = HitboxDmgMultiplier * EnemyDamage;
        }

        // 부위별 보너스 적용
        if (HitboxComp == HitboxOwner->HeadColComponent)
        {
            Damage *= HeadshotMultiplier;
        }

        // 에어샷 판정 적용
        if (HitboxOwner->IsInAir())
        {
            Damage *= DmgMultOnAirshot;
        }

        // 거리 배율 적용
        float TargetShooterDist = FVector::Dist(HitboxOwner->GetActorLocation(), InitialLocation);
        if (TargetShooterDist <= TR_GUN_CLOSE_DIST)
        {
            Damage *= DmgMultDistClose;
        }
        else if (TargetShooterDist >= TR_GUN_LONG_DIST)
        {
            Damage *= DmgMultDistFar;
        }

        // 거리 Falloff 적용
        if (bHasDmgDistFallOff)
        {
            Damage *= TRUtils::GetFallOffMultOfDist(HitboxOwner->GetDistanceTo(GetInstigator()), DmgDistFallOffMult, GunConst::GUN_MAX_FALLOFF_DIST, GunConst::GUN_MIN_FALLOFF_DIST);
        }
    }

    UGameplayStatics::ApplyPointDamage(HitboxOwner, Damage, NormalImpulse, Hit, GetInstigator()->Controller, GetInstigator()/* 이 투사체를 생성한 폰 */, DamageType);
}

void ABaseProjectile::Local_InitializeVFX()
{
    Local_SpawnProjVFX();
    Local_SpawnTrailVFX();
    // 필요 시 추가
}

void ABaseProjectile::Local_CleanUpVFX()
{
    Local_DestroyProjVFX();
    Local_DestroyTrailVFX();
    // 필요 시 추가
}

void ABaseProjectile::Local_SpawnProjVFX()
{
    if (ProjectileVFX && ProjectileMeshComponent && !Local_ProjInst)
    {
        Local_ProjInst = UNiagaraFunctionLibrary::SpawnSystemAttached(
            ProjectileVFX,
            ProjectileMeshComponent,
            FName(NAME_None),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true,
            true,
            ENCPoolMethod::AutoRelease,
            true
        );
    }
}

void ABaseProjectile::Local_DestroyProjVFX()
{
    if (Local_ProjInst)
    {
        Local_ProjInst->DeactivateImmediate();
    }
}

void ABaseProjectile::Local_SpawnTrailVFX()
{
    if (TrailVFX && ProjectileMeshComponent && !Local_TrailInst)
    {
        Local_TrailInst = UNiagaraFunctionLibrary::SpawnSystemAttached(
            TrailVFX,
            ProjectileMeshComponent,
            FName(NAME_None),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true,
            true,
            ENCPoolMethod::AutoRelease,
            true
        );
    }
}

void ABaseProjectile::Local_DestroyTrailVFX()
{
    if (Local_TrailInst)
    {
        Local_TrailInst->Deactivate();
    }
}

void ABaseProjectile::Server_SpawnProjHitObjectVFX(FVector HitLocation, FRotator HitRotation)
{
    if (HasAuthority())
    {
        Multicast_SpawnProjHitObjectVFX(HitLocation, HitRotation);
    }
}

void ABaseProjectile::Server_SpawnProjHitCharacterVFX(FVector HitLocation, FRotator HitRotation)
{
    if (HasAuthority())
    {
        Multicast_SpawnProjHitCharacterVFX(HitLocation, HitRotation);
    }
}

void ABaseProjectile::Multicast_SpawnProjHitObjectVFX_Implementation(FVector HitLocation, FRotator HitRotation)
{
    if (HitObjectImpactVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            HitObjectImpactVFX,
            HitLocation,
            HitRotation,
            FVector(1.0f),
            true,
            true,
            ENCPoolMethod::AutoRelease,
            true
        );
    }
}

void ABaseProjectile::Multicast_SpawnProjHitCharacterVFX_Implementation(FVector HitLocation, FRotator HitRotation)
{
    if (HitCharacterImpactVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            HitCharacterImpactVFX,
            HitLocation,
            HitRotation,
            FVector(1.0f),
            true,
            true,
            ENCPoolMethod::AutoRelease,
            true
        );
    }
}

