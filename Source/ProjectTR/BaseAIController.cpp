// Copyright (C) 2024 by Haguk Kim


#include "BaseAIController.h"
#include "BotCharacter.h"
#include "BaseTargetPoint.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "TRMacros.h"
#include "TRUtils.h"
#include "FPSCharacter.h"
#include "TRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/CrowdFollowingComponent.h"

ABaseAIController::ABaseAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
#pragma region /** Component Initialization */
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception"));
	if (AIPerception) 
	{
		AIPerception->OnPerceptionUpdated.AddDynamic(this, &ABaseAIController::UpdatePerception);
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::UpdateTargetPerception);

		// 감각정보 기본값 설정
		SightSense = UAISense_Sight::StaticClass();
	}
#pragma endregion
}

void ABaseAIController::PauseAILogic()
{
	// 컨트롤러 데이터 리셋
	ClearControllerData();

	// 중지 로직
	SetActorHiddenInGame(true);
	if (PerceptionComponent)
	{
		PerceptionComponent->Deactivate();
		PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
		PerceptionComponent->ForgetAll(); // 현재 인지중인 정보들만 지울 뿐 타깃 정보는 블랙보드에 남아있음에 유의
	}
	if (BrainComponent)
	{
		BrainComponent->StopLogic(FString("ABaseAIController::PauseAILogic"));
		BrainComponent->Deactivate();
	}
}

void ABaseAIController::HaltAILogic()
{
	// 중지
	PauseAILogic();

	// 블랙보드 데이터 리셋
	ClearBlackboardValues();
}

void ABaseAIController::StartAILogic()
{
	// 시작 로직
	SetActorHiddenInGame(false);
	if (PerceptionComponent)
	{
		PerceptionComponent->Activate();
		PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
	}
	if (BrainComponent)
	{
		BrainComponent->Activate();
		BrainComponent->StartLogic();
	}
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();

	UBlackboardComponent* BBComp = Blackboard.Get();
	if (UseBlackboard(BlackboardAsset, BBComp))
	{
		if (IsValid(BehaviorTreeAsset))
		{
			RunBehaviorTree(BehaviorTreeAsset);
		}
	}
}

void ABaseAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (IsValid(Blackboard.Get()) && IsValid(BehaviorTreeAsset))
	{
		Blackboard->InitializeBlackboard(*(BehaviorTreeAsset->BlackboardAsset));
	}
}

void ABaseAIController::UpdatePerception(const TArray<AActor*>& Actors)
{
	if (!IsValid(Blackboard)) return;
	OnPerceptionUpdate(Actors);
}

void ABaseAIController::UpdateTargetPerception(AActor* Actor, FAIStimulus Stimulus)
{
	if (!IsValid(Blackboard)) return;
	if (!IsValid(Actor)) return;

	if (Stimulus.Type == GetSightSenseID())
	{
		HandleTargetSightSense(Actor, Stimulus);
	}
	// else if ...
}

void ABaseAIController::OnPerceptionUpdate(const TArray<AActor*>& Actors)
{
	return; // 필요 시 오버라이드
}

void ABaseAIController::HandleTargetSightSense(AActor* Actor, FAIStimulus Stimulus)
{
	// 유효한 공격 타깃에 대해서만 로직 처리
	// TODO: 추후 공격 이외의 AI 퍼셉션 기반 타깃팅이 추가될 경우 수정 필요
	if (!IsAttackTargetValid(Actor)) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		// 현재 타깃이 없는 경우 타깃 설정
		if (!IsValid(CurrentTarget))
		{
			ClearTargetLastLocation();
			ClearAttackerLastLocation();
			SetTargetAs(Actor);
		}
		else
		{
			// TODO
		}
	}
	else
	{
		// 시야에서 벗어난 경우 타깃 해제
		if (Actor == CurrentTarget)
		{
			ClearTarget();
			SetTargetLastLocationOf(Actor);
		}
	}
	return;
}

bool ABaseAIController::IsAttackTargetValid(AActor* Target)
{
	if (!IsValid(Target)) return false;

	// TODO: 타겟팅 세분화
	AFPSCharacter* TargetChar = Cast<AFPSCharacter>(Target);
	if (IsValid(TargetChar))
	{
		// 사망 시 타깃팅하지 않음
		if (TargetChar->GetHasDied()) return false;
		return true;
	}
	// 게임 캐릭터가 아닌 경우 타깃이 아님
	return false;
}

bool ABaseAIController::UpdateCurrTargetValidity()
{
	// 타겟이 사망 등의 모종의 이유로 더이상 유효하지 않은 경우
	if (!IsValid(CurrentTarget) || !IsAttackTargetValid(CurrentTarget))
	{
		ClearTarget();
		return false;
	}
	return true;
}

void ABaseAIController::SetTargetAs(AActor* Target)
{
	Blackboard->SetValueAsObject(TEXT(AI_TARGET), Target);
	CurrentTarget = Target;
	SetFocus(Target, EAIFocusPriority::Gameplay); // 이후 모든 액션에서 타깃을 바라보도록 설정한다
}

void ABaseAIController::ClearTarget()
{
	Blackboard->SetValueAsObject(TEXT(AI_TARGET), nullptr);
	CurrentTarget = nullptr;
	ClearFocus(EAIFocusPriority::Gameplay);
}

void ABaseAIController::SetTargetLastLocationOf(AActor* Target)
{
	if (!Target)
	{
		UE_LOG(LogTemp, Error, TEXT("SetTargetLastLocationOf - Target is invalid!"));
		return;
	}

	// 타깃이 갑자기 높게 점프를 해서 시야에서 벗어나는 등의 상황에서도 추적을 하려면,
	// 액터 지점을 그대로 쓰는 대신 적절한 네브메쉬 지점을 찾아야 한다
	FVector TargetEstimatedPoint;
	bool bFound = TRUtils::GetMovablePointNearActor(GetWorld(), Target, TEXT("BotCapsule"), TR_DIST_CLOSE_TO_PLAYER_HEIGHT * 8, TR_DIST_CLOSE_TO_PLAYER_HEIGHT, TargetEstimatedPoint);
	if (!bFound) return;

	////TESTING
	DrawDebugSphere(GetWorld(), TargetEstimatedPoint, 100, 10, FColor::Purple, false, 5.0f);

	Blackboard->SetValueAsVector(TEXT(AI_TARGET_LAST_KNOWN_LOCATION), TargetEstimatedPoint);
	Blackboard->SetValueAsBool(TEXT(AI_IS_TARGET_LAST_KNOWN_LOC_VALID), true);
}

void ABaseAIController::ClearTargetLastLocation()
{
	Blackboard->SetValueAsVector(TEXT(AI_TARGET_LAST_KNOWN_LOCATION), FVector(0, 0, 0));
	Blackboard->SetValueAsBool(TEXT(AI_IS_TARGET_LAST_KNOWN_LOC_VALID), false);
}

void ABaseAIController::SetAttackerLastLocationAs(FVector Location)
{
	Blackboard->SetValueAsVector(TEXT(AI_ATTACKER_LAST_KNOWN_LOCATION), Location);
	Blackboard->SetValueAsBool(TEXT(AI_IS_ATTACKER_LAST_KNOWN_LOC_VALID), true);
}

void ABaseAIController::ClearAttackerLastLocation()
{
	Blackboard->SetValueAsVector(TEXT(AI_ATTACKER_LAST_KNOWN_LOCATION), FVector(0, 0, 0));
	Blackboard->SetValueAsBool(TEXT(AI_IS_ATTACKER_LAST_KNOWN_LOC_VALID), false);
}

void ABaseAIController::SetPatrolLocationAs(FVector Location)
{
	Blackboard->SetValueAsVector(TEXT(AI_PATROL_LOCATION), Location);
}

void ABaseAIController::ClearPatrolLocation()
{
	Blackboard->SetValueAsVector(TEXT(AI_PATROL_LOCATION), FVector(0, 0, 0));
}

bool ABaseAIController::SetTargetWithinSight()
{
	if (PerceptionComponent)
	{
		TArray<AActor*> PerceivedActors;
		PerceptionComponent->GetCurrentlyPerceivedActors(SightSense, PerceivedActors);

		for (AActor* Perceived : PerceivedActors)
		{
			if (IsAttackTargetValid(Perceived))
			{
				ClearTargetLastLocation();
				ClearAttackerLastLocation();
				SetTargetAs(Perceived);
				return true;
			}
		}
	}
	return false;
}

void ABaseAIController::OnDamageTaken(AActor* AttackedFrom, int32 Damage)
{
	// 유효한 적대 가능한 대상이 0 이상 데미지를 주었을 경우에만 공격자로 취급함
	// 0 초과가 아닌 이상임에 유의
	if (IsAttackTargetValid(AttackedFrom) && Damage >= 0)
	{
		// 현재 저장된 공격자 정보가 없을 경우에만 새 정보 등록
		if (Blackboard->GetValueAsBool(TEXT(AI_IS_ATTACKER_LAST_KNOWN_LOC_VALID)))
		{
			return;
		}
		SetAttackerLastLocationAs(AttackedFrom->GetActorLocation());
	}
}

void ABaseAIController::OnAggroStart()
{
	ABotCharacter* Bot = Cast<ABotCharacter>(GetPawn());
	if (Bot)
	{
		UBaseCharacterMovementComponent* TRMoveComp = Bot->GetTRCharacterMovementComponent();
		if (TRMoveComp)
		{
			TRMoveComp->OnInput_SprintStart();
		}
	}
}

void ABaseAIController::OnAggroEnd()
{
	ABotCharacter* Bot = Cast<ABotCharacter>(GetPawn());
	if (Bot)
	{
		UBaseCharacterMovementComponent* TRMoveComp = Bot->GetTRCharacterMovementComponent();
		if (TRMoveComp)
		{
			TRMoveComp->OnInput_SprintStop();
		}
	}
}

FAISenseID ABaseAIController::GetSightSenseID()
{
	return UAISense::GetSenseID(SightSense);
}

void ABaseAIController::ClearControllerData()
{
	CurrentTarget = nullptr;
	ClearFocus(EAIFocusPriority::Gameplay);
}

void ABaseAIController::ClearBlackboardValues()
{
	if (Blackboard)
	{
		UBlackboardData* BlackboardData = Blackboard->GetBlackboardAsset();
		if (BlackboardData == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("ClearBlackboardValues - BlackboardData is null."));
			return;
		}

		for (const FBlackboardEntry& Entry : BlackboardData->Keys)
		{
			Blackboard->ClearValue(Entry.EntryName);
		}
	}
}

void ABaseAIController::PrintDebug() const
{
	// 필요에 맞게 사용
	FVector Vec1 = Blackboard->GetValueAsVector(TEXT(AI_PATROL_LOCATION));
	FVector Vec2 = Blackboard->GetValueAsVector(TEXT(AI_TARGET));
	FVector Vec3 = Blackboard->GetValueAsVector(TEXT(AI_TARGET_LAST_KNOWN_LOCATION));
	FVector Vec4 = Blackboard->GetValueAsVector(TEXT(AI_ATTACKER_LAST_KNOWN_LOCATION));
	bool bBool1 = Blackboard->GetValueAsBool(TEXT(AI_IS_ATTACKER_LAST_KNOWN_LOC_VALID));
	bool bBool2 = Blackboard->GetValueAsBool(TEXT(AI_IS_TARGET_LAST_KNOWN_LOC_VALID));
	bool bBool3 = Blackboard->GetValueAsBool(TEXT(AI_IS_MELEE_ANIM_PLAYING));
	bool bBool4 = Blackboard->GetValueAsBool(TEXT(AI_IS_RANGED_ANIM_PLAYING));
	UE_LOG(LogTemp, Error, TEXT("id:%s BBaddr:%d BTaddr:%d TargetLocX:%f"),
		*(GetName()), Blackboard, BrainComponent, Vec2.X
	);
}
