// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "UserSpecificWidgetActor.generated.h"

/*
* 월드의 특정 위치에 위치하면서 동시에 특정 유저에게만 보이는 범용 위젯에 사용되는 액터의 인터페이스이다
* 그러나 만약 기존에 존재하는 다른 특정 액터와 연결성이 있는 경우(e.g. 네임태그)에는 
* 이 액터를 사용하기보다는 해당 액터 내에 직접 컴포넌트를 추가하는 것이 더 권장된다.
* 
* 이 클래스는 인터페이스 역할을 하므로 직접 사용할 수 없다
*/
UCLASS(Abstract)
class PROJECTTR_API AUserSpecificWidgetActor : public AActor
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		// 최초 1회만 레플리케이션한다
		DOREPLIFETIME_CONDITION(AUserSpecificWidgetActor, TargetViewers, COND_InitialOnly);
	}

public:	
	AUserSpecificWidgetActor();

protected:
	virtual void BeginPlay() override;

public:
	// 특정 호스트에 대해 visibility를 설정한다
	virtual void Local_SetVisibility(bool bVisibility);

	// 추후 풀링 사용할 경우 호출
	virtual void ClearData();

	// 위젯을 볼 수 있는 호스트를 추가한다
	virtual void Server_AddTarget(APlayerController* Target) final;

	virtual UWidgetComponent* GetWidgetComp() final { return Cast<UWidgetComponent>(RootComponent); }

protected:
	// 서버에서 최초 액터를 생성할 때 1회 초기화되며, 그 이후로는 변경되지 않는다
	// 이 값은 레플리케이션 되지는 않지만, 서버에서 액터를 생성할 때 할당된 값이 클라이언트에 전달된다
	UPROPERTY(Replicated)
	TArray<APlayerController*> TargetViewers;
};
