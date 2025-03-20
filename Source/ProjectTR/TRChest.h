// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "MuzzleTriggeredActor.h"
#include "MimicBot.h"
#include "TRStructs.h"
#include "Net/UnrealNetwork.h"
#include "TRChest.generated.h"


/**
 * 미믹과 상자는 별도로 분리한다
 */
UCLASS()
class PROJECTTR_API ATRChest : public AMuzzleTriggeredActor
{
	GENERATED_BODY()

public:
	ATRChest();
	void BeginPlay() override;
	virtual void Initialize() override;
	virtual void OnMuzzleTriggered(class AGameCharacter* TriggeredBy);

/* Components */
public:
	// 상자 애니메이션 처리를 위해 스켈레탈 메시를 사용한다
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;

#pragma region /** Gameplay */
public:
	// 상자를 개봉을 시도한다
	// 아이템 보상을 얻거나 미믹을 소환하는 로직의 진입 지점
	void Server_TryOpenChest();

/* 미믹 */
protected:
	// 미믹을 생성한다
	void Server_TurnIntoMimic();

protected:
	// 상자를 열었을 때 미믹으로 변이할 확률 [0,1]
	// NOTE: 미믹 변이 여부는 BeginPlay 호출 시점에 미리 정해진다
	UPROPERTY(EditAnywhere, Category = "Gameplay")
	float MimicChance = 0.0f;

	// 미믹으로 변이 시 소환할 캐릭터 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TSubclassOf<AMimicBot> MimicClass = nullptr;

	// 미믹인지 여부; BeginPlay 호출 시점에 정해진다
	// 상자 애니메이션을 통해 미믹인지 판별이 가능하다
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	bool bServer_IsMimic = false;

/* 보상 */
protected:
	// 상자를 개봉한다
	void Server_OpenChest();

	// 보상을 선택해 드랍한다
	void Server_DropRewards();

	// 적절한 보상을 선택해 등록한다
	// 이 함수는 이미 상자 인스턴스가 레벨 내에 소환되었음을 가정한 채 처리된다
	void Server_AutoInitRewards();

	// 주어진 콘텍스트에 따라 이 상자에 리워드 자동 생성 시 몇 개의 아이템 후보를 생성할지 결정한다
	// 주의: 반환 개수는 아이템 "후보의 개수"이므로, 각 후보들의 실제 스폰 레이트에 따라 실질적인 드랍 아이템 수는 달라질 수 있다
	int32 Server_AutoSelectMaxRewardCount(int32 DungeonDepth) const;

protected:
	// 보상으로 주어질 수 있는 아이템들과 각각의 드랍 확률
	// 고정된 보상을 제공하기 위해 이 값을 직접 수정할 수 있다
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FDropItem> DropRewards;

	// 상자의 개봉 여부
	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly)
	bool bServer_ChestOpened = false;

	// 이 값이 참일 경우 클래스 디폴트에 등록된 DropRewards 정보를 무시하고 
	// 대신 이 인스턴스가 속한 콘텍스트에 맞는 리워드를 자동으로 랜덤하게 선택해 배정한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	bool bAutoGenReward = false;

	// 보상 항목 자동 생성 시 사용할 후보들
	// 만약 bAutoGenReward이 false라면 이 값은 상자 보상에 아무런 영향을 주지 않는다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	TArray<FUnfilteredDropItem> AutoGenDropCandidates;
#pragma endregion

#pragma region /** Animation / VFX */
protected:
	// 모든 호스트들에게 미믹 생성 시 VFX를 재생한다
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMimicSpawnVFX();

	// 상자 흔들기 재생
	void Server_PlayShakeAnim();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayShakeAnim();

	// 상자 개봉 재생
	void Server_PlayOpenAnim();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOpenAnim();

protected:
	// 미믹인 경우 상자 흔들기 애니메이션을 일정 시간 재생하는 것으로 플레이어에게 힌트를 준다
	// 이 값이 길 수록 미믹인지 확실하게 판별하기 위해 소요되는 시간이 길어진다
	// 즉 플레이어에게 미믹으로부터의 안전을 대가로 던전 시간을 소모할지 결정하는 것을 강요한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ShakeInterval = 40.0f;

	// 상자 흔들기 타이머
	FTimerHandle ShakeTimer;

protected:
	// 미믹 소환 시 VFX
	// 시야를 가려 상자를 없애고 미믹이 생성되는 과정을 자연스럽게 연결한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraSystem* MimicSpawnVFX = nullptr;

	// 상자 개봉 시 VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_OpenAnim = nullptr;

	// 상자 흔들기 VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> AM_ShakeAnim = nullptr;
#pragma endregion
};
