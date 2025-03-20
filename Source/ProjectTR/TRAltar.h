// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "DungeonActor.h"
#include "TRAltar.generated.h"

UCLASS()
class PROJECTTR_API ATRAltar : public ADungeonActor
{
	GENERATED_BODY()
	
public:
	ATRAltar();

protected:
	virtual void BeginPlay() override;

	// 콜리전에 바인딩할 함수
	UFUNCTION()
	void Server_OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// 아이템 감지용 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<class UBoxComponent> DetectionComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	TObjectPtr<class USceneComponent> SpawnPointComponent = nullptr;

protected:
	// 아이템 감지 시 처리 로직
	// 성공적으로 아이템이 제단에서 소비되었을 경우 true를 반환한다
	bool Server_OnItemDetection(class ABaseItem* Item);
	bool Server_OnTokenDetection(class ATRToken* Token);
	bool Server_OnGunDetection(class AGunItem* Gun);
	bool Server_OnSoulDetection(class ATRSoul* Soul);

	// 스폰 위치 정보
	FVector GetSpawnLocation();
	FRotator GetSpawnRotation();

	// 제단 사용 후 로직 처리
	void Server_PostAltarUsage();

	// 제단 파괴
	void Server_DestroyAltar();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DestroyAltar();

	// 제단 파괴 시 처리할 VFX 로직
	// 필요 로직은 블루프린트에서 오버라이드해 처리한다
	UFUNCTION(BlueprintImplementableEvent)
	void Local_OnAltarDestruction();

protected:
	// 제단 사용 횟수
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 UseCount = 0;

	// 제단 파괴 여부
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	bool bAltarDestroyed = false;

	// 현재 제단 사용 시 파괴 가능성
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	float CurrDestructionChance = 0.0f;

	// 제단에 토큰 드랍 시 제단 파괴 가능성 증가치
	// 이 값은 토큰의 종류에 따라 배율이 붙거나 확률이 더해지는 등 변경될 수 있다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	float AddDestructionChance_Token = 0.2f;

	// 제단에 영혼석 드랍 시 제단 파괴 가능성 증가치
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	float AddDestructionChance_Soul = 0.5f;
};
