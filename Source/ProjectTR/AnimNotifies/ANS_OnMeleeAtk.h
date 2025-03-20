// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_OnMeleeAtk.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTTR_API UANS_OnMeleeAtk : public UAnimNotifyState
{
	GENERATED_BODY()

	UANS_OnMeleeAtk();
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	// 소켓 위 기준 히트 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SocketHitRadius = 40.0f;

	// 히트 판정을 할 오브젝트 타입 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TEnumAsByte<EObjectTypeQuery>> HitObjectTypes;

	// 근접 공격 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRightHandMelee = true;

	// 이번 노티파이 시퀀스에서 처리중인 히트 타깃(게임 캐릭터로 한정)들의 목록과 관련 정보
	TMap<class AGameCharacter*, FHitResult> HitTargetsForCurrSequence;
};
