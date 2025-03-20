// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "Components/ActorComponent.h"
#include "RangedAttackComponent.generated.h"

/*
* 무기 장착과 무관하게 무언가를 발사해야 할 경우 사용한다
*/
UCLASS()
class PROJECTTR_API URangedAttackComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	URangedAttackComponent();

protected:
	virtual void BeginPlay() override;

/* Logic */
public:
	UFUNCTION(BlueprintCallable)
	void Server_FireProjectile(const FVector& SpawnLocation, const FVector& TargetLocation);
	
protected:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnFireProjectile();

private:
	// 발사할 투사체를 선택한다
	// 기본적으로 0번 엘리먼트를 반환하며, 별도의 세부 로직이 필요할 경우 오버라이드 해 구현한다
	virtual TSubclassOf<ABaseProjectile> SelectFireProjectile();

protected:
	// 발사할 투사체들의 목록
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ABaseProjectile>> FireProjectileClass;
};
