// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "IconStageActor.generated.h"

UCLASS()
class PROJECTTR_API AIconStageActor : public AActor
{
	GENERATED_BODY()

public:
#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AIconStageActor, DisplayedActor);
	}
#pragma endregion
	
public:
	AIconStageActor();

	// 파괴 시 처리 로직
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 무대에 세울 액터를 등록하고 정확한 위치에 배치한다
	// 그 후 액터의 크기에 맞게 캡처 카메라 위치를 조정한다
	void SetupDisplayActor(class AActor* ActorToDisplay, FRotator DeltaDisplayRot);

	// Getter
	const TWeakObjectPtr<AActor> GetDisplayedActor() const { return DisplayedActor; }

	// 아이콘 렌더 타깃 생성
	class UTextureRenderTarget2D* CreateIconRenderTarget(int32 Width, int32 Height);

	// 텍스처 타깃 설정
	void SetTextureTargetAs(class UTextureRenderTarget2D* Target);

	// 현재 설정된 타겟 캡처
	void CaptureTarget();

	// 렌더 타깃 Getter
	class UTextureRenderTarget2D* GetRenderTarget() const;

protected:
	// 아이템 액터를 배치할 위치를 반환한다
	FVector GetItemStageLocation() const;

	// 아이템 액터를 배치할 때 사용할 회전를 반환한다
	FRotator GetItemStageRotation() const;

	// 캡쳐하려는 대상의 가로 세로 크기에 따라 카메라 위치를 조정한다
	void AdjustCameraOnTarget(float TargetWidth, float TargetHeight);

protected:
	virtual void BeginPlay() override;

protected:
	// 현재 디스플레이 중인 액터
	UPROPERTY(Replicated)
	TObjectPtr<AActor> DisplayedActor = nullptr;

public:
	// 아이콘 디스플레이용 카메라 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USceneCaptureComponent2D* CaptureComponent = nullptr;

	// 디스플레이할 액터를 소환할 위치 및 방향
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UArrowComponent* DisplayComponent = nullptr;

	// 이 액터의 아이콘 생성이 완료되었는지 여부
	bool bLocal_HasIconGenerated = false;
};
