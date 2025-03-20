// Copyright (C) 2024 by Haguk Kim


#include "SpikeTrap.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DamageTypeNeutral.h"

ASpikeTrap::ASpikeTrap()
{
    PrimaryActorTick.bCanEverTick = true;
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetCollisionProfileName(TEXT("TREnvironment"));
    CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASpikeTrap::OnOverlapBegin);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &ASpikeTrap::OnOverlapEnd);

	check(MeshComponent != nullptr);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 가시가 관통하는 효과를 주기 위함
}

void ASpikeTrap::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ASpikeTrap::OnTriggered()
{
	Super::OnTriggered();
	bTrapTriggered = true;
}

void ASpikeTrap::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComp && OverlappedComp == CollisionBox)
	{
		OnBoxCollision(OtherActor);
	}
}

void ASpikeTrap::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 필요 시 추가
}

void ASpikeTrap::OnBoxCollision(AActor* Target)
{
	AGameCharacter* GameTarget = Cast<AGameCharacter>(Target);
	if (GameTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameCharacter touched the spike trap: %s"), *GameTarget->GetName());
		ApplyTrapDamage(GameTarget);
	}
}

void ASpikeTrap::ApplyTrapDamage(AGameCharacter* Target)
{
	if (!Target) return;
	if (bDamageOnlyPlayers && !Cast<AFPSCharacter>(Target)) return;
	if (FMath::Abs(TrapDamage) > UE_SMALL_NUMBER)
	{
		const FVector& TrapLoc = GetActorLocation();
		const FVector& TargetLoc = Target->GetActorLocation();
		FHitResult TrapHitRes;
		TrapHitRes.ImpactNormal = TrapLoc - TargetLoc;
		TrapHitRes.ImpactPoint = TargetLoc;
		TrapHitRes.TraceStart = TrapLoc;
		TrapHitRes.Location = TargetLoc;
		UGameplayStatics::ApplyPointDamage(
			Target, 
			TrapDamage, 
			TargetLoc - TrapLoc, 
			TrapHitRes,
			Target->GetController(), 
			this, 
			DamageType
		);
	}
}
