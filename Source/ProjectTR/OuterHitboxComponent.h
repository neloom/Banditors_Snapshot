// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "OuterHitboxComponent.generated.h"

/**
 * 히트박스 최적화를 위해 사용하는 외곽 히트박스
 * UHitboxComponent와는 아무런 hierarchical한 연결이 없다
 * 외곽 히트박스에 오버랩이 발생할 경우 내부 히트박스 활성화를 처리한다
 * 이는 캐릭터의 불필요한 히트박스 트랜스폼 연산을 매 틱마다 처리하는 부하를 제거하기 위한 장치이다
 */
UCLASS()
class PROJECTTR_API UOuterHitboxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UOuterHitboxComponent();

	// 총알 라인트레이스, 투사체 오버랩 등 내부 히트박스들을 활성화시키는 이벤트 발생 시 호출
	// 히트 이벤트 발생 시 직접 호출해주어야 한다
	// 자동으로 호출되도록 바인딩하지 않는 이유는, LineTrace 등의 경우 히트 결과를 반환은 하지만 실제 히트 이벤트가 발생하지는 않기 때문이다
	// NOTE: 0 이하의 값을 전달할 경우 1틱 동안만 활성화한다
	void OnOuterHitboxCollision(float HitboxDuration);
};
