// Copyright (C) 2024 by Haguk Kim


#include "RandomChoiceActor.h"
#include "ProjectTRGameModeBase.h"

void ARandomChoiceActor::BeginPlay()
{
	Super::BeginPlay();
}

void ARandomChoiceActor::Initialize()
{
	Super::Initialize();
	if (HasAuthority())
	{
		if (FMath::FRand() <= GenerationChance)
		{
			Server_SelectAndReplaceWithRandomCand(true);
		}
	}
}

ARandomChoiceActor::ARandomChoiceActor()
{
	// 특성 상 콜리전이 없어야 함
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

ADungeonActor* ARandomChoiceActor::Server_SelectAndReplaceWithRandomCand(bool bApplyGravity)
{
	if (!HasAuthority()) return nullptr;
	if (SpawnCandidates.IsEmpty())
	{
		Destroy();
		return nullptr;
	}

	// 후보 배열과 확률 배율의 개수가 안맞을 경우 
	// 만약 후보가 더 많다면 잘린 액터들 부터는 소환 확률을 0으로 취급한다
	// 만약 weight가 더 많다면 weight를 잘라 축소한다
	int32 CandCount = SpawnCandidates.Num();
	int32 WeightCount = SpawnChances.Num();
	if (CandCount != WeightCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectAndReplaceWithRandomCand - Number of candidates: %d, Number of weights: %d"), SpawnCandidates.Num(), SpawnChances.Num());
		
		if (CandCount > WeightCount)
		{
			for (int32 Fill = 0; Fill < CandCount - WeightCount; ++Fill)
			{
				SpawnChances.Add(0.0f);
			}
		}
		else
		{
			for (int32 Fill = 0; Fill < WeightCount - CandCount; ++Fill)
			{
				SpawnChances.Pop();
			}
		}
	}

	// 랜덤 선택
	// 인자가 <T*> 형태이므로 맞춰준다
	TArray<TSubclassOf<ADungeonActor>*> Elements;
	for (int32 CandIdx = 0; CandIdx < SpawnCandidates.Num(); ++CandIdx)
	{
		Elements.Add(&SpawnCandidates[CandIdx]);
	}
	TSubclassOf<ADungeonActor>* ChosenCand = TRUtils::GetRandomElementByWeight<TSubclassOf<ADungeonActor>>(Elements, SpawnChances);

	if (!ChosenCand)
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectAndReplaceWithRandomCand - Failed to randomly select the actor subclass"));
		Destroy();
		return nullptr;
	}

	// 소환
	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (TRGM)
		{
			FVector SpawnLocation = this->GetActorLocation();
			if (bApplyGravity)
			{
				SpawnLocation = GetFootLocation(this->GetActorLocation());
			}

			ADungeonActor* SpawnedActor = TRGM->SpawnDungeonActor(*ChosenCand, World, SpawnLocation, this->GetActorRotation(), FActorSpawnParameters());
			this->Destroy();
			if (SpawnedActor) return SpawnedActor;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("SelectAndReplaceWithRandomCand - Something went wrong!"));
	return nullptr;
}

FVector ARandomChoiceActor::GetFootLocation(FVector RootLocation, float MaxLineLength)
{
	FVector TraceEnd = RootLocation - FVector(0, 0, MaxLineLength);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.bTraceComplex = true; // 정밀하게 판정

	GetWorld()->LineTraceSingleByChannel(HitResult, RootLocation, TraceEnd, ECC_WorldStatic, CollisionParams);
	if (HitResult.bBlockingHit)
	{
		return HitResult.Location;
	}
	return RootLocation;
}
