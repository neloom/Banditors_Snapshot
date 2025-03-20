// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "MuzzleTriggeredActor.h"
#include "StatusEffect.h"
#include "TRShrine.generated.h"

UENUM(BlueprintType)
enum class EShrineAffectType : uint8
{
	SAT_None UMETA(DisplayName = "Affect no one"),
	SAT_InvokerOnly UMETA(DisplayName = "Affect the invoker only"),
	SAT_InvokerAndNearbyAllies UMETA(DisplayName = "Affect the invoker and its allies nearby"),
	SAT_InvokerAndAllies UMETA(DisplayName = "Affect the invoker and its allies regardless of the distance"),
	SAT_NearbyEnemies UMETA(DisplayName = "Affect the invoker's enemies nearby"),
	SAT_AllEnemies UMETA(DisplayName = "Affect the invoker's enemies regardless of the distance"),
};


/**
 * 특수한 로직이 꼭 필요한 경우가 아니라면 C++ 상속 대신 블루프린트 상속을 사용해 구현한다
 */
UCLASS()
class PROJECTTR_API ATRShrine : public AMuzzleTriggeredActor
{
	GENERATED_BODY()
	
public:
	ATRShrine();
	void BeginPlay() override;
	virtual void OnMuzzleTriggered(class AGameCharacter* TriggeredBy) override;

protected:
	UFUNCTION()
	virtual void OnShrineTriggered(class AGameCharacter* TriggeredBy);

	// 추가적인 로직이 필요할 경우 이 함수를 Implement해 작성한다
	UFUNCTION(BlueprintNativeEvent)
	void ShrineLogic(class AGameCharacter* TriggeredBy);
	virtual void ShrineLogic_Implementation(class AGameCharacter* TriggeredBy);

	// 매터리얼 초기화
	void InitMaterials();

	// 사용 가능 여부 설정
	void Server_SetShrineState(bool bUsable);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetShrineState(bool bUsable);

	// shrine 사용 여부에 따른 시각효과 변경
	void SetMatToUsable();
	void SetMatToUnusable();

	void SetLightToUsable();
	void SetLightToUnusable();

protected:
	// 초기 활성화 상태 여부
	UPROPERTY(EditAnywhere)
	bool bDefaultUsability = true;
	bool bCurrentUsability = true;

	// 일회성으로 사용되는지 여부
	UPROPERTY(EditDefaultsOnly)
	bool bUseOnce = true;

	// shrine이 트리거될 경우 어떤 캐릭터(들)에게 상태이상을 부여하는지 여부
	UPROPERTY(EditDefaultsOnly)
	EShrineAffectType StatusEffectApplyType = EShrineAffectType::SAT_None;

	// 이 shrine으로부터 얼마까지 떨어진 거리의 캐릭터에게까지 영향을 주는지 최대 거리
	UPROPERTY(EditDefaultsOnly)
	float TargetAffectDistance = 0.0f;

	// Trigger 시 유효 타깃(들)에게 부여할 상태이상
	UPROPERTY(EditDefaultsOnly)
	TArray<FStatEffectGenInfo> StatEffAppliedToTargets;

/* VFX */
public:
	TObjectPtr<UMaterialInstanceDynamic> GemMaterialInst = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UPointLightComponent> GemLightComp = nullptr;

protected:
	// 보석 매터리얼 인덱스 번호; 메쉬에 따라 직접 설정해주어야 한다
	UPROPERTY(EditAnywhere)
	int32 GemMaterialIndex = 0;

	// 파라미터 이름
	UPROPERTY(EditDefaultsOnly)
	FString BaseColorParamName = "BaseColor";

	UPROPERTY(EditDefaultsOnly)
	FString LightColorParamName = "LightColor";

	UPROPERTY(EditDefaultsOnly)
	FString BrightnessParamName = "Brightness";

	// 매터리얼 기본값 지정
	UPROPERTY(EditDefaultsOnly)
	FLinearColor BaseColorUsable;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor LightColorUsable;

	UPROPERTY(EditDefaultsOnly)
	float BrightnessUsable = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor BaseColorUnusable;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor LightColorUnusable;

	UPROPERTY(EditDefaultsOnly)
	float BrightnessUnusable = 0.0f;

	// 라이트 기본값 지정
	// NOTE: 색상의 경우 매터리얼 light color와 동일한 값 사용
	float LightIntensityUsable = 100.0f;
};
