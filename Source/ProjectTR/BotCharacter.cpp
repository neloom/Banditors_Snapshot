// Copyright (C) 2024 by Haguk Kim


#include "BotCharacter.h"
#include "Components/CapsuleComponent.h"
#include "RangedAttackComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BaseCharacterMovementComponent.h"
#include "FPSCharacter.h"
#include "ProjectTRGameModeBase.h"
#include "AnimConfig.h"
#include "BaseAIController.h"
#include "AIControllerPool.h"
#include "TRMacros.h"
#include "TRUtils.h"
#include "TRToken.h"

ABotCharacter::ABotCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp != nullptr);
	CapsuleComp->SetCollisionProfileName("BotCapsule"); // 봇의 캡슐 콜리전 설정은 플레이어와 차이가 있음

	RangeAtkComp = CreateDefaultSubobject<URangedAttackComponent>(TEXT("BotRangeAttack"));
	check(RangeAtkComp != nullptr);
	RangeAtkComp->SetupAttachment(CapsuleComp);

	// 봇 메쉬 최적화 옵션
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (SkeletalMesh)
	{
		SkeletalMesh->SetCollisionProfileName("BotSkelMesh");

		SkeletalMesh->bDisableMorphTarget = true;
		SkeletalMesh->SetAllowAnimCurveEvaluation(true);

		SkeletalMesh->bPerBoneMotionBlur = false;
		SkeletalMesh->bSkipKinematicUpdateWhenInterpolating = true;
		SkeletalMesh->bSkipBoundsUpdateWhenInterpolating = true;
		SkeletalMesh->bComponentUseFixedSkelBounds = true;

		SkeletalMesh->SkipUpdateOverlapsOptimEnabled = true;

		SkeletalMesh->bEnableUpdateRateOptimizations = true;
		SkeletalMesh->OnAnimUpdateRateParamsCreated.BindLambda(
			[](FAnimUpdateRateParameters* Param)
			{
				// NOTE: 애니메이션 재생 레이트를 수정해 최적화한다
				Param->bShouldUseLodMap = true;

				// NOTE:
				// LOD별 스킵 프레임을 설정한다
				// 60fps의 상황에서 값이 2일경우 30fps로 처리한다
				Param->MaxEvalRateForInterpolation = 24;
				Param->LODToFrameSkipMap.Add(0, 2);
				Param->LODToFrameSkipMap.Add(1, 2);
				Param->LODToFrameSkipMap.Add(2, 2);
				Param->LODToFrameSkipMap.Add(3, 2);

				// 시야 바깥의 적에 대해서는 낮은 값을 사용
				Param->BaseNonRenderedUpdateRate = 4;
			}
		);
	}

	// 애니메이션 최적화
	// 봇의 밀리 공격이 애니메이션 기반으로 처리되기 때문에, 서버는 가장 비싼 옵션이 불가피하다
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// 부드러운 AI 이동을 위한 기본값 설정 변경
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	UBaseCharacterMovementComponent* MoveComp = Cast<UBaseCharacterMovementComponent>(GetCharacterMovement());
	if (MoveComp)
	{
		MoveComp->bUseControllerDesiredRotation = true; // SetFocus 사용을 위함
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->RotationRate = FRotator(0, 180, 0);

		MoveComp->WalkMaxWalkSpeed = 400.0f;
		MoveComp->SprintMaxWalkSpeed = 800.0f;

		// 최적화를 위해 이동 과정중 발생하는 피직스 연산 해제
		MoveComp->bEnablePhysicsInteraction = false;

		MoveComp->bAlwaysCheckFloor = false;

		// 무브먼트의 정확도를 대가로 성능 개선
		// 필요 시 적절한 중간값으로 조정
		MoveComp->BrakingSubStepTime = 0.05f; // max 0.05; 클수록 성능 상승
		MoveComp->MaxSimulationTimeStep = 0.5f; // max 0.5; 클수록 성능 상승
		MoveComp->MaxSimulationIterations = 1; // min 1; 작을수록 성능 상승

		MoveComp->PrimaryComponentTick.TickInterval = TR_BOT_MOVECOMP_TICKRATE;
	}
	
	if (!DropTokenClass)
	{
		// 토큰 클래스에 바인딩된 값이 없을 경우 기본값 설정
		static ConstructorHelpers::FClassFinder<ATRToken> TokenClassFinder(TEXT(DEFAULT_TOKEN_ITEM));
		if (TokenClassFinder.Succeeded())
		{
			DropTokenClass = TokenClassFinder.Class;
		}
	}
}

void ABotCharacter::Deactivate()
{
	bBotActive = false;

	USkeletalMeshComponent* BotMeshComp = GetMesh();
	if (BotMeshComp && BotMeshComp->IsActive())
	{
		BotMeshComp->Deactivate();
		BotMeshComp->SetVisibility(false); // 서버사이드 렌더링 부하 감소
		BotMeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	}
	ABaseAIController* AIController = Cast<ABaseAIController>(GetController());
	if (AIController)
	{
		// Halt가 아닌 Pause중이기 때문에 AI에 잔여 데이터가 남아있음에 유의
		AIController->PauseAILogic();
	}
	TR_PRINT("DEACTIVATE!");
}

void ABotCharacter::Activate()
{
	bBotActive = true;

	USkeletalMeshComponent* BotMeshComp = GetMesh();
	if (BotMeshComp && !BotMeshComp->IsActive())
	{
		BotMeshComp->Activate(false);
		BotMeshComp->SetVisibility(true); // 서버사이드 렌더링 부하 감소
		BotMeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
	ABaseAIController* AIController = Cast<ABaseAIController>(GetController());
	if (AIController)
	{
		AIController->StartAILogic();
	}
	TR_PRINT("ACTIVATE!");
}

TPair<FVector, FRotator> ABotCharacter::GetMuzzleInfo()
{
	if (!RangeAtkComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ABotCharacter::GetMuzzleInfo - Invalid Virtual muzzle comp!"));
		return TPair<FVector, FRotator>(GetActorLocation(), GetActorRotation());
	}
	return TPair<FVector, FRotator>(RangeAtkComp->GetComponentLocation(), RangeAtkComp->GetComponentRotation());
}

void ABotCharacter::BeginPlay()
{
	// NOTE: GameCharacter에서 클라이언트의 경우 시야 밖 메쉬 본 연산을 해제
	Super::BeginPlay();
}

void ABotCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// CPU skinning 해제
	// 게임 특성 상 CPU 보틀넥이 훨씬 더 크기 때문에 해제한다
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (SkeletalMesh)
	{
		if (SkeletalMesh->GetSkinnedAsset() && SkeletalMesh->GetSkinnedAsset()->GetResourceForRendering())
		{
			SkeletalMesh->SetCPUSkinningEnabled(false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABotCharacter::PostInitializeComponents - Failed to enable CPU skinning for %s."), *(this->GetName()));
		}
	}
}

void ABotCharacter::BeginDestroy()
{
	Super::BeginDestroy();
}

float ABotCharacter::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	ABaseAIController* AIController = Cast<ABaseAIController>(GetController());
	if (AIController && EventInstigator)
	{
		AIController->OnDamageTaken(EventInstigator->GetPawn(), DamageTaken);
	}

	// 실제 데미지 연산을 마지막에 처리한다
	return Super::TakeDamage(DamageTaken, DamageEvent, EventInstigator, DamageCauser);
}

void ABotCharacter::Server_ProcessDeath()
{	
	Super::Server_ProcessDeath(); // 공격자에 대한 경험치 처리는 여기서 처리됨

	if (!HasAuthority()) return;

	// 봇의 경우 리워드를 같이 드랍한다
	Server_SpawnAndDropRewardsAt(this->GetActorLocation());

	// 컨트롤러를 반환한다
	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* TRGameMode = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (TRGameMode)
		{
			TRGameMode->AIPool->Inanimate(this);
		}
	}
}

void ABotCharacter::Multicast_ProcessDeath()
{
	Super::Multicast_ProcessDeath();
}

UAnimMontage* ABotCharacter::GetAnimMontage(EBotAnimType Type, uint8 Index)
{
	if (!AnimConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("GetAnimMontage - Invalid character animconfig! : %s"), *(GetName()));
		return nullptr;
	}
	switch (Type)
	{
		case EBotAnimType::BAT_MELEE:
		{
			switch (Index)
			{
				default:
					return nullptr;
				case 0:
					return AnimConfig->AM_MeleeAtk1;
				case 1:
					return AnimConfig->AM_MeleeAtk2;
			}
			break;
		}
		case EBotAnimType::BAT_RANGED:
		{
			switch (Index)
			{
			default:
				return nullptr;
			case 0:
				return AnimConfig->AM_RangedAtk1;
			case 1:
				return AnimConfig->AM_RangedAtk2;
			}
			break;
		}
	}
	return nullptr;
}

bool ABotCharacter::IsTargetInMeleeAtkTrialRange(const AActor* Target) const
{
	if (Target)
	{
		return GetDistanceTo(Target) <= MeleeAttackTrialRange;
	}
	return false;
}

void ABotCharacter::EndBotMeleeAttackAnim(UAnimMontage* Montage, bool bInterrupted)
{
	if (HasAuthority())
	{
		bServer_IsBotMeleeAttacking = false;
	}
	else
	{
		// 필요 시 클라 로직 구현
	}
}

bool ABotCharacter::IsAimingAtTarget(const AActor* Target)
{
	// 타깃이 없으면 성공한 것으로 취급
	if (!Target) return true;

	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = false;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(Target);

	// 머즐과 타깃 사이를 확인
	TPair<FVector, FRotator> MuzzleInfo = GetMuzzleInfo();
	float TargetDist = GetDistanceTo(Target) - KINDA_SMALL_NUMBER; // 타깃 약간 앞까지 확인; 타깃은 트레이스 대상에서 제외되므로 크게 의미는 없음
	UWorld* World = GetWorld();
	if (World)
	{
		FHitResult HitResult;
		const FVector& LineStart = MuzzleInfo.Get<0>();
		const FVector& LineEnd = LineStart + (TargetDist * MuzzleInfo.Get<1>().Vector().GetSafeNormal());
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, LineStart, LineEnd, ECC_Projectile, TraceParams);

		DrawDebugLine(World, LineStart, LineEnd, FColor::Red, true, -1, 0, 1.0f);

		return !bHit;
	}
	return false;
}

bool ABotCharacter::IsTargetInRangedAtkRange(const AActor* Target) const
{
	if (Target)
	{
		return GetDistanceTo(Target) <= RangedAttackRange;
	}
	return false;
}

void ABotCharacter::EndBotRangedAttackAnim(UAnimMontage* Montage, bool bInterrupted)
{
	if (HasAuthority())
	{
		bServer_IsBotRangedAttacking = false;
	}
	else
	{
		// 필요 시 클라 로직 구현
	}
}

void ABotCharacter::Multicast_PlayBotRangedAttackAnim_Implementation(uint8 AnimIndex)
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (SkeletalMesh && AnimConfig)
	{
		if (UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance())
		{
			UAnimMontage* Montage = GetAnimMontage(EBotAnimType::BAT_RANGED, AnimIndex);
			if (!Montage)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cannot find BotRangedAttack AnimMontage."));
				return;
			}

			if (HasAuthority())
			{
				// 서버 로직
				bServer_IsBotRangedAttacking = true;
			}

			// 재생 후 바인딩
			AnimInst->Montage_Play(Montage);

			// 매번 새롭게 바인딩
			FOnMontageEnded RangedAtkMontageEnded;
			RangedAtkMontageEnded.BindUObject(this, &ABotCharacter::EndBotRangedAttackAnim);
			AnimInst->Montage_SetEndDelegate(RangedAtkMontageEnded, Montage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Multicast_PlayBotRangedAttackAnim - Invalid mesh / AnimConfig. Please check the blueprint : %s"), *(GetName()));
	}
}

void ABotCharacter::Server_BotRangedAttack_Implementation()
{
	if (!HasAuthority()) return;
	// TODO: 애니메이션 종류 변경 로직 추가
	// FIXME: 애니메이션이 null일 수 있음 (종류가 한가지인 경우)
	Multicast_PlayBotRangedAttackAnim(0);

	ABaseAIController* AIController = Cast<ABaseAIController>(GetController());
	if (AIController && RangeAtkComp)
	{
		AActor* CurrentTarget = AIController->GetCurrentTarget();
		if (IsValid(CurrentTarget))
		{
			RangeAtkComp->Server_FireProjectile(GetMuzzleInfo().Get<0>(), CurrentTarget->GetActorLocation());
		}
	}
}

void ABotCharacter::Server_GiveKillExpRewardTo(AFPSCharacter* Player)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_GiveKillExpRewardTo - Client should not call this function. Aborting!"));
		return;
	}
	if (IsValid(Player))
	{
		if (IsValid(Player->ExpComp))
		{
			Player->ExpComp->GainExp(this->KillExpReward);
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Server_GiveKillExpRewardTo - %s Failed to give exp reward"), *(this->GetName()));
}

void ABotCharacter::Server_SpawnAndDropRewardsAt(FVector Location)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_SpawnAndDropRewardsAt - Client should not call this function. Aborting!"));
		return;
	}

	AProjectTRGameModeBase* TRGameMode = nullptr;
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_SpawnAndDropRewardsAt - Invalid world! This could result in lost drops."));
		return;
	}

	TRGameMode = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
	if (!IsValid(TRGameMode))
	{
		UE_LOG(LogTemp, Error, TEXT("Server_SpawnAndDropRewardsAt - Cannot get valid game mode!"));
		return;
	}

	// 로직
	const TArray<FDropItem>& DropItems = Server_SelectDropItems();
	for (const FDropItem& Loot : DropItems)
	{
		FVector2D XYOffset = FMath::RandPointInCircle(RewardDropRandOffset);
		Server_AddDeferredSpawnItem(Loot.ItemRef, FVector(Location.X + XYOffset.X, Location.Y + XYOffset.Y, Location.Z));
	}
}

void ABotCharacter::Server_OnDeathHandleLastAttacker(AGameCharacter* Attacker)
{
	Super::Server_OnDeathHandleLastAttacker(Attacker);

	// 던전이 현재 레드 모드가 아닐 경우 공격자 경험치 획득
	// NOTE: 무한 파밍을 방지하기 위함
	UWorld* World = GetWorld();
	if (World)
	{
		AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
		if (TRGM && !TRGM->IsDungeonRedMode())
		{
			AFPSCharacter* AttackPlayerChar = Cast<AFPSCharacter>(Attacker);
			if (IsValid(AttackPlayerChar) && AttackPlayerChar->ExpComp)
			{
				int32 PrevLevel = AttackPlayerChar->ExpComp->GetLevel();
				Server_GiveKillExpRewardTo(AttackPlayerChar);
				int32 NewLevel = AttackPlayerChar->ExpComp->GetLevel();

				// 공격자가 이 공격으로 인해 레벨이 상승했을 경우 봇의 위치에 토큰을 드랍한다
				if (PrevLevel < NewLevel)
				{
					bool bTokenGenerated = false;
					if (DropTokenClass)
					{
						bTokenGenerated = (TRGM->SpawnToken(DropTokenClass, World, GetActorLocation(), GetActorRotation(), FActorSpawnParameters(), TRGM->RandomizeTokenTier(NewLevel)) != nullptr);
					}
					if (!bTokenGenerated)
					{
						UE_LOG(LogTemp, Error, TEXT("ABotCharacter::Server_OnDeathHandleLastAttacker - Token generation failed! DropTokenClass validity: %d"), (DropTokenClass != nullptr));
					}
				}
			}
		}
		else
		{
			//TEMP
			TR_PRINT("The dungeon level is in red mode, you cannot earn exp anymore!");
		}
	}
	return;
}

const TArray<FDropItem> ABotCharacter::Server_SelectDropItems()
{
	TArray<FDropItem> Result;
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_SelectDropItems - Client should not call this function. Returning an empty array!"));
		return Result;
	}
	return TRUtils::SelectDropItems(this->DropRewards);
}

void ABotCharacter::Server_BotMeleeAttack_Implementation()
{
	if (!HasAuthority()) return;
	// TODO: 밀리 애니메이션 종류 변경 로직 추가
	// FIXME: 애니메이션이 null일 수 있음 (종류가 한가지인 경우)
	Multicast_PlayBotMeleeAttackAnim(FMath::Rand() % 2);
}

void ABotCharacter::Multicast_PlayBotMeleeAttackAnim_Implementation(uint8 AnimIndex)
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (SkeletalMesh && AnimConfig)
	{
		if (UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance())
		{
			UAnimMontage* Montage = GetAnimMontage(EBotAnimType::BAT_MELEE, AnimIndex);
			if (!Montage)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cannot find BotMeleeAttack AnimMontage."));
				return;
			}

			if (HasAuthority())
			{
				// 서버 로직
				bServer_IsBotMeleeAttacking = true;
			}

			// 재생 후 바인딩
			AnimInst->Montage_Play(Montage);

			// 매번 새롭게 바인딩
			FOnMontageEnded MeleeAtkMontageEnded;
			MeleeAtkMontageEnded.BindUObject(this, &ABotCharacter::EndBotMeleeAttackAnim);
			AnimInst->Montage_SetEndDelegate(MeleeAtkMontageEnded, Montage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Multicast_PlayBotMeleeAttackAnim - Invalid mesh / AnimConfig. Please check the blueprint : %s"), *(GetName()));
	}
}
