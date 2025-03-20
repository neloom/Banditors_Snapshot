// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BaseCharacterMovementComponent.generated.h"


UENUM(BlueprintType)
enum class EHumanoidLandState : uint8
{
	NORMAL UMETA(DisplayName = "Normal"),
	SOFT UMETA(DisplayName = "Soft"),
	HEAVY UMETA(DisplayName = "Heavy"),
};

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Slide			UMETA(DisplayName = "Slide"),
	CMOVE_MAX			UMETA(Hidden),
};

UCLASS()
class PROJECTTR_API UBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_TR : public FSavedMove_Character
	{
	public:
		enum CompressedFlags
		{
			FLAG_Sprint = 0x10,
			FLAG_Custom_1 = 0x20,
			FLAG_Custom_2 = 0x40,
			FLAG_Custom_3 = 0x80,
		};

		uint8 Saved_bWantsToSprint : 1;

		FSavedMove_TR();

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_TR : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_TR(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	// 커스텀 무브먼트 관련 파라미터
	UPROPERTY(EditDefaultsOnly) 
	float SprintMaxWalkSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly) 
	float WalkMaxWalkSpeed = 825.0f;

protected:
	// 리플레이 캐시
	bool bWantsToSprint = false;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual bool IsMovingOnGround() const override;
	// MOD2

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

public:
	// 네트워크 동기화 로직 인풋의 처리 진입점
	void OnInput_SprintStart();
	void OnInput_SprintStop();
	void OnInput_CrouchStart();
	void OnInput_CrouchStop();
	
public:
	UBaseCharacterMovementComponent();
	virtual void BeginPlay() override;

	// 캐릭터를 반환한다
	class AGameCharacter* GetTROwner();

#pragma region /** Networking */
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UBaseCharacterMovementComponent, DeltaMaxWalkSpeedCached);

		// TODO: 이하 값들은 봇의 경우 굳이 레플리케이션 하지 않아도 됨
		DOREPLIFETIME(UBaseCharacterMovementComponent, DeltaJumpCountCached);
		DOREPLIFETIME(UBaseCharacterMovementComponent, DeltaJumpSpeedCached);
		DOREPLIFETIME(UBaseCharacterMovementComponent, DeltaRollSpeedCached);
		DOREPLIFETIME(UBaseCharacterMovementComponent, DeltaRollDelayCached);
	}

/* RPCs */
public:
	UFUNCTION(Server, Reliable)
	void Server_RegisterRollRPC(float Forward, float Right);
#pragma endregion

#pragma region /** Animation caches */
// NOTE: 애니메이션 재생에 필요한 게임플레이 값들의 캐시본으로, 게임 로직에는 영향을 주지 않는다
protected:
	// 회전 속도
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float TurnRateCached = 0.0f;

	// 이동 방향
	UPROPERTY(BlueprintReadOnly, Category = "Moving")
	float MovingForwardCached = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Moving")
	float MovingRightCached = 0.0f;
#pragma endregion

#pragma region /** Defaults */
// 게임플레이 시작 시 초기화되는 값들로, BeginPlay 시점 업데이트된다 
// 이 값은 이후 게임플레이에서 이 무브먼트 인스턴스의 기본값으로 사용된다
// 에디터에서 기본값을 수정할 경우, 캐릭터 컴포넌트 내의 프로퍼티를 직접 수정하면 BeginPlay에서 해당 값들로 업데이트된다
protected:
	float DefaultJumpZVelocity = 900.0f;
	float DefaultAirControl = 0.2f;

	float DefaultMaxWalkSpeed = 900.0f;
	float DefaultMinAnalogWalkSpeed = 300.0f;
	float DefaultMaxWalkSpeedCrouched = 200.0f;

	float DefaultMaxAcceleration = 12000.0f;
	float DefaultBrakingDecelerationWalking = 12.0f;
	float DefaultGroundFriction = 6.0f;

	float DefaultGravityScale = 2.6f;
	float DefaultMass = 70.0f;

	float DefaultSlideMinEnterSpeed = 1000.0f;
	float DefaultSlideOnEnterImpulse = 300.0f;
	float DefaultSlideGravityForce = 4800.0f;
	float DefaultSlideFriction = 0.4f;
	float DefaultSlideStrafePower = 0.5f;

	float DefaultRollSpeed = 1200.f;
	float DefaultRollZAccelMult = 4.0f;
	float DefaultRollDelay = 0.7f;
#pragma endregion

#pragma region /** Status Delta Sum */
public:
	// Setters
	void SetDeltaMaxWalkSpeedCached(float Value);
	void SetDeltaJumpCountCached(int32 Value);
	void SetDeltaJumpSpeedCached(float Value);
	void SetDeltaRollSpeedCached(float Value);
	void SetDeltaRollDelayCached(float Value);

	void ResetDeltaMaxWalkSpeedCached();
	void ResetDeltaJumpCountCached();
	void ResetDeltaJumpSpeedCached();
	void ResetDeltaRollSpeedCached();
	void ResetDeltaRollDelayCached();

private:
	UPROPERTY(Replicated)
	float DeltaMaxWalkSpeedCached = 0.0f;

	UPROPERTY(Replicated)
	int32 DeltaJumpCountCached = 0;

	UPROPERTY(Replicated)
	float DeltaJumpSpeedCached = 0.0f;

	UPROPERTY(Replicated)
	float DeltaRollSpeedCached = 0.0f;

	UPROPERTY(Replicated)
	float DeltaRollDelayCached = 0.0f;
#pragma endregion

#pragma region /** Logic */
/* Crouching / Sliding */
protected:
	// 슬라이딩 중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "Rolling")
	bool bIsSliding = false;

	UPROPERTY(EditDefaultsOnly)
	float SlideMinEnterSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly)
	float SlideOnEnterImpulse = 300.f;

	UPROPERTY(EditDefaultsOnly)
	float SlideGravityForce = 4800.f;

	UPROPERTY(EditDefaultsOnly)
	float SlideFriction = 0.4f;

	UPROPERTY(EditDefaultsOnly)
	float SlideStrafePower = 0.5f;

public:
	UPROPERTY(EditDefaultsOnly)
	bool bCanSlide = false;

protected:
	bool CanSlideNow() const;
	void EnterSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit) const;

/* Jumping */
protected:
	// 로직
	virtual bool DoJump(bool bReplayingSimulatedInput) override;

/* Roll */
protected:
	// 구르기 중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "Rolling")
	bool bIsRolling = false;

	// 구르기 속도
	// 거리와 속력에 영향을 준다
	UPROPERTY(EditDefaultsOnly, Category = "Rolling")
	float RollSpeed = 1200.0f;

	// 구르기 시 Z 가속도
	// 질량에 이 값을 곱한만큼의 가속도가 부여된다
	UPROPERTY(EditDefaultsOnly, Category = "Rolling")
	float RollZAccelMult = 4.0f;

	// 구르기 쿨타임
	UPROPERTY(EditDefaultsOnly, Category = "Rolling")
	float RollDelay = 0.7f;

private:
	// 구르기 쿨타임 타이머
	FTimerHandle RollCooldownTimer;

protected:
	// 구르기 시작
	void DoStartRolling(float Forward, float Right);

	// 구르기 종료
	void DoEndRolling();

private:
	// 캐릭터 구르기 로직
	void RollTowards(const FVector& Direction);

/* Landing */
public:
	// 착지 상태
	UPROPERTY(BlueprintReadWrite, Category = "Landing")
	EHumanoidLandState TRLandState = EHumanoidLandState::NORMAL;

protected:
	// 착지 타입 별 속도 하한선 지정
	UPROPERTY(BlueprintReadWrite, Category = "Landing")
	float HeavyLandingSpeed = 1200.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Landing")
	float NormalLandingSpeed = 900.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Landing")
	float SoftLandingSpeed = 300.0f;

public:
	// 착지 상태 변경
	void UpdateLandingState();
#pragma endregion

#pragma region /** Utils */
protected:
	// 상대적 벡터의 x(전방), y(우측)가 주어졌을 경우 그 벡터가 캐릭터 전방을 기준으로 몇도 기울어진 값인지 계산한다
	double GetInputDirectionAngleFromForward(float Forward, float Right);

	// 현재 특정 커스텀 무브먼트 모드 사용중인지 여부
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

/* Getters, Setters */
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetTurnRate() { return TurnRateCached; }
	FORCEINLINE void SetTurnRate(float Value) { TurnRateCached = Value; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetMovingForward() { return MovingForwardCached; }
	FORCEINLINE void SetMovingForward(float Value) { MovingForwardCached = Value; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetMovingRight() { return MovingRightCached; }
	FORCEINLINE void SetMovingRight(float Value) { MovingRightCached = Value; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsSprinting() { return bWantsToSprint; }
	FORCEINLINE void SetIsSprinting(bool Value) { bWantsToSprint = Value; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsRolling() { return bIsRolling; }
	FORCEINLINE void SetIsRolling(bool Value) { bIsRolling = Value; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsSliding() { return bIsSliding; }
	FORCEINLINE void SetIsSliding(bool Value) { bIsSliding = Value; }
#pragma endregion
};