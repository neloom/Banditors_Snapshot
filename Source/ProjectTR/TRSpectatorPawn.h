// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpectatorPawn.h"
#include "InputActionValue.h"
#include "TRStructs.h"
#include "TRSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ATRSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

	ATRSpectatorPawn();

protected:
	virtual void Tick(float DeltaTime) override;
	virtual FBox GetComponentsBoundingBox(bool bNonColliding, bool bIncludeFromChildActors) const override;

#pragma region /** Networking */
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ATRSpectatorPawn, SpectatingTarget);
	}

	// 관전 중인 타깃 변경 시
	UFUNCTION()
	void OnRep_SpecTargetChange();
#pragma endregion

#pragma region /** Input */
public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 로컬 플레이어에게 InputMappingContext를 추가한다
	void AddLocalPlayerInputMappingContext(const class UInputMappingContext* Context, int32 Priority, bool bClearAllMappings);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<class UInputMappingContext> InputMapping = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<class UInputConfig> InputConfig = nullptr;
#pragma endregion

#pragma region /** Action */
protected:
	// 다음 플레이어로 관전 화면 전환
	void SpecChangeNext(const FInputActionValue& Value);

	// 이전 플레이어로 관전 화면 전환
	void SpecChangePrev(const FInputActionValue& Value);
#pragma endregion

public:
	// 캐릭터 클래스 정보 및 인스턴스 정보 저장; 추후 리스폰 시 사용될 정보
	void Server_SetOriginalCharacterData(class AGameCharacter* Character);

	// 다음 혹은 이전 타깃으로 관전 대상을 변경한다
	// Direction true일 경우 다음, false 일 경우 이전을 의미한다
	UFUNCTION(Server, Reliable)
	void Server_ChangeSpecTargetRPC(bool Direction);

	// 리스폰을 처리한다
	// 호출 시 Destroy가 호출되어 다음 틱에 이 폰은 파괴된다
	UFUNCTION(Server, Reliable)
	void Server_RespawnPlayerRPC();

	// 기존 타깃과 동일한지 확인하고 추가 로직을 수행하는 Setter
	void SetSpectatingTarget(ACharacter* NewTarget);

protected:
	// 타깃이 기존과 달라졌을 경우 호출하는 함수
	// 호스트에 따른 로직 분기를 처리
	void OnSpecTargetChange();

	// 새 유효한 관전 타깃이 설정되었을 경우
	void Server_OnNewSpecTargetSet();
	void Client_OnNewSpecTargetSet();

	// 관전할 수 있는 대상이 없을 경우
	void Server_OnNoSpecTargetExist();
	void Client_OnNoSpecTargetExist();

	// 새 대상에게 뷰타겟이 설정되었을 때 필요 시 추가 작업을 수행한다
	// 공통 로직
	void Local_OnViewTargetSet();

public:
	/* Getters */
	class ACharacter* Local_GetSpectatingTarget() { return SpectatingTarget; }

protected:
	// Optional
	// 리스폰 할 캐릭터 액터의 클래스의 Path를 저장
	TSubclassOf<class AFPSCharacter> RespawnPlayerClass = nullptr;

	// 리스폰할 캐릭터 액터 인스턴스에 적용할 정보 저장
	UPROPERTY(BlueprintReadOnly)
	FGameCharacterInstanceData RespawnPlayerInstanceData;

	// 현재 관전중인 대상
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SpecTargetChange, Category = "Spectation")
	class ACharacter* SpectatingTarget = nullptr;

	// 바로 이전 관전했던 대상
	class ACharacter* PrevSpectatingTarget = nullptr;
};
