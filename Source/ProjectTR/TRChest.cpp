// Copyright (C) 2024 by Haguk Kim


#include "TRChest.h"
#include "ProjectTRGameModeBase.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AIControllerPool.h"
#include "TRUtils.h"

ATRChest::ATRChest()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	check(SkeletalMeshComponent != nullptr);
	SkeletalMeshComponent->SetupAttachment(RootComponent);
}

void ATRChest::BeginPlay()
{
	Super::BeginPlay();
}

void ATRChest::Initialize()
{
	Super::Initialize();
	// 미믹 여부 결정
	if (HasAuthority())
	{
		if (FMath::SRand() <= MimicChance)
		{
			bServer_IsMimic = true;

			// 미믹일 경우 일정 주기로 상자 흔들리는 애니메이션 재생
			GetWorldTimerManager().SetTimer(ShakeTimer, this, &ATRChest::Server_PlayShakeAnim, ShakeInterval, true);
		}
		else
		{
			bServer_IsMimic = false;
		}

		if (bAutoGenReward)
		{
			Server_AutoInitRewards();
		}
	}
}

void ATRChest::OnMuzzleTriggered(AGameCharacter* TriggeredBy)
{
	Super::OnMuzzleTriggered(TriggeredBy);
	Server_TryOpenChest();
}

void ATRChest::Server_TryOpenChest()
{
	if (bServer_ChestOpened)
	{
		return;
	}
	bServer_ChestOpened = true;

	if (bServer_IsMimic)
	{
		Server_TurnIntoMimic();
	}
	else
	{
		Server_OpenChest();
	}
}

void ATRChest::Server_TurnIntoMimic()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_TurnIntoMimic - Unexpected error!"));
		return;
	}
	if (!MimicClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_TurnIntoMimic - Mimic class is null!"));
		return;
	}

	AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(GetWorld()->GetAuthGameMode());
	if (TRGM && TRGM->AIPool)
	{
		AMimicBot* Mimic = Cast<AMimicBot>(TRGM->SpawnBot(MimicClass, World, GetActorLocation()/*TODO*/, GetActorRotation()/*TODO*/, FActorSpawnParameters()));
		if (Mimic)
		{
			// 미믹은 상자와 같은 아이템들을 처치보상으로 제공
			Mimic->SetupChestInfo(this->DropRewards);

			TRGM->AIPool->Animate(Mimic, true/*정규 스폰 과정이 아니므로 새 컨트롤러 생성 강제*/);

			// VFX 재생
			if (MimicSpawnVFX)
			{
				Multicast_PlayMimicSpawnVFX();
			}

			// 상자 파괴
			Destroy();
		}
	}
}

void ATRChest::Server_OpenChest()
{
	Server_PlayOpenAnim();
	Server_DropRewards();
}

void ATRChest::Server_DropRewards()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World) return;

	TArray<FDropItem> ChosenRewards = TRUtils::SelectDropItems(DropRewards);
	AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
	if (TRGM)
	{
		FVector DropLocation = GetActorLocation() + (GetActorForwardVector() * 50.f);
		for (const FDropItem& Loot : ChosenRewards)
		{
			ABaseItem* DroppedItem = TRGM->SpawnItem(Loot.ItemRef, World, DropLocation, FRotator(), FActorSpawnParameters());
		}
	}
}

void ATRChest::Server_AutoInitRewards()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World) return;
	AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
	if (TRGM)
	{
		int32 DungeonDepth = TRGM->GetCurrentDungeonDepth();
		int32 MaxRewardCount = Server_AutoSelectMaxRewardCount(DungeonDepth);

		// 후보들을 다시 동적으로 선택한다
		DropRewards.Empty();
		DropRewards = TRUtils::FilterAndSelectCandidates(AutoGenDropCandidates, DungeonDepth, MaxRewardCount);
	}
}

int32 ATRChest::Server_AutoSelectMaxRewardCount(int32 DungeonDepth) const
{
	// TEMP TODO DEBUG
	// TODO: 벨런스 조정
	return 3;
}

void ATRChest::Server_PlayShakeAnim()
{
	if (!HasAuthority()) return;
	Multicast_PlayShakeAnim();
}

void ATRChest::Server_PlayOpenAnim()
{
	if (!HasAuthority()) return;
	Multicast_PlayOpenAnim();
}

void ATRChest::Multicast_PlayOpenAnim_Implementation()
{
	UAnimInstance* AnimInst = SkeletalMeshComponent->GetAnimInstance();
	if (AnimInst && AM_OpenAnim)
	{
		AnimInst->Montage_Play(AM_OpenAnim);
	}
}

void ATRChest::Multicast_PlayShakeAnim_Implementation()
{
	UAnimInstance* AnimInst = SkeletalMeshComponent->GetAnimInstance();
	if (AnimInst && AM_ShakeAnim)
	{
		AnimInst->Montage_Play(AM_ShakeAnim);
	}
}

void ATRChest::Multicast_PlayMimicSpawnVFX_Implementation()
{
	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		MimicSpawnVFX,
		GetActorLocation(),
		GetActorRotation(),
		GetActorScale3D() * 3.0f,
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true
	);
}
