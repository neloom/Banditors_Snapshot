// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonActor.generated.h"

/*
* 던전 내에 생성되는 액터들 중 아이템과 캐릭터를 제외한
* 특수한 로직을 가져야 하는 액터들의 로직을 처리하기 위해 사용된다
* 레버, 버튼, 함정과 같이 어떤 트리거를 발동시키는 액터도 될 수 있고
* 철창 같이 트리거에 의해 발동되는 액터도 될 수 있다
* 
* 모든 로직은 Server-Authoritative하게 처리된다
*/
UCLASS(BlueprintType, Blueprintable)
class PROJECTTR_API ADungeonActor : public AActor
{
	GENERATED_BODY()
	
public:
	ADungeonActor();
	// NOTE: 초기화 로직은 가급적 Initialize에서 작성할 것
	virtual void BeginPlay() override;
	virtual void Initialize();

	// 방 기준 벡터(relative offset)를 실제 던전 월드 기준 좌표로 변환한다
	// 던전 제너레이션 과정을 거치지 않고 생성된 액터들의 경우 사용하는 의미가 없는 함수이다
	// WARNING: 정수값으로 다운캐스팅 되므로 소수부 값은 날아갈 수 있다
	FVector ConvertRoomToWorld(const FVector RoomVector);

/* Gameplay */
public:
	// 이 액터의 트리거를 발동시킨다
	// 다른 액터에 의해 호출될 수 있다
	UFUNCTION(BlueprintCallable)
	void TriggerThis();

	// 이 액터의 모든 페어에 대해 트리거를 발동시킨다
	UFUNCTION(BlueprintCallable)
	void TriggerPairs();

	// 이 액터의 페어들 중 특정 인덱스 페어에 대해 트리거를 발동시킨다
	// 해당 인덱스가 유효하지 않은 경우 아무 것도 처리하지 않는다
	UFUNCTION(BlueprintCallable)
	void TriggerPair(int Index = 0);

	// 이 액터가 영향을 줄 수 있는 페어 액터를 추가한다
	// 이렇게 추가된 Pair는 TriggerPairs() 호출 시 전부 TriggerThis() 된다.
	void AddPairActor(ADungeonActor* Pair);

protected:
	// 이 액터의 트리거가 발동했을 때의 로직이다
	// virtual 함수이므로 C++ 구현만 허용한다
	UFUNCTION()
	virtual void OnTriggered();

	// BP에서 구현 가능한 트리거 함수로, 필요 시 OnTriggered()를 필요에 따라 별도로 호출해주어야 한다
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnTriggeredImplementable();

public:
	// 이 던전액터가 속하는 룸
	// 이 값은 통상적인 절차 생성 과정을 거쳐 생성된 던전 액터에게만 부여된다
	// 즉, 던전 생성 완료 이후 추가로 던전액터를 생성하는 경우 (TRGM의 Spawn함수와 같은 경우) 이 값은 nullptr일 수 있다
	// 때문에 이 값을 해당 액터가 속한 방을 찾는 목적으로 사용하는 것은 권장되지 않으며,
	// 초기화 시점 방과 월드 사이 좌표 변환을 처리하는 목적 정도로만 사용하는 게 권장된다
	class URoom* OwningRoom = nullptr;

public:
	// 일반적으로 던전 제너레이션 과정에서 Initialize가 호출이 되지만,
	// 던전 제너레이터가 없는 레벨이거나, 아니면 던전 생성 완료 후 생성된 액터들의 경우 이를 수동으로 호출해주어야 한다
	// 이때 사용 가능한 플래그 값이다
	UPROPERTY(EditInstanceOnly)
	bool bForceInitDuringBeginPlay = false;

	// 보스전 클리어 시 자동으로 트리거해야 하는지 여부
	// 만약 액터가 속한 던전이 보스 던전이 아니라면 아무 기능도 하지 않는다
	UPROPERTY(EditInstanceOnly)
	bool bTriggerOnBossFightClear = false;

protected:
	// 이 액터가 영향을 줄 수 있는 페어 액터들의 목록
	UPROPERTY(EditInstanceOnly)
	TArray<TWeakObjectPtr<ADungeonActor>> Pairs;

private:
	// 초기화는 한 차례만 진행되어야 함; 이를 검증하기 위한 플래그
	bool bInitialized = false;

/* Components */
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USceneComponent> SceneComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent = nullptr;
};
