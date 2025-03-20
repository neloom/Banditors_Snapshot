// Copyright (C) 2024 by Haguk Kim


#include "IconStageActor.h"
#include "TRMacros.h"
#include "BaseItem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ArrowComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"

// Sets default values
AIconStageActor::AIconStageActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 아이콘 액터는 특성 상 굉장히 먼 거리에 스폰되는데, 이때도 레플리케이션이 처리되야 한다
	bReplicates = true;
	NetCullDistanceSquared = MAX_FLT;

	if (!DisplayComponent)
	{
		DisplayComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	}

	if (!RootComponent)
	{
		RootComponent = DisplayComponent;
	}

	if (!CaptureComponent)
	{
		CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComp"));

		// 프로퍼티
		CaptureComponent->ProjectionType = ECameraProjectionMode::Type::Orthographic;
		CaptureComponent->FOVAngle = 60.0f;
		CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList; // 타깃 아이템 액터만 렌더링한다
		CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
		CaptureComponent->CompositeMode = ESceneCaptureCompositeMode::SCCM_Overwrite;
		CaptureComponent->MaxViewDistanceOverride = 1024.f;
		CaptureComponent->DetailMode = EDetailMode::DM_High;

		CaptureComponent->bAutoActivate = false;
		CaptureComponent->bCaptureOnMovement = false;
		CaptureComponent->bCaptureEveryFrame = false;

		CaptureComponent->SetupAttachment(RootComponent);
	}

	SetActorEnableCollision(false);
	bReplicates = true; // 레플리케이션 사용
}

void AIconStageActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 파괴 시 바인딩 된 액터도 함께 파괴
	if (HasAuthority() && IsValid(DisplayedActor))
	{
		DisplayedActor->Destroy();
	}
}

void AIconStageActor::SetupDisplayActor(AActor* ActorToDisplay, FRotator DeltaDisplayRot)
{
	if (IsValid(DisplayedActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupDisplayActor - DisplayedActor is valid. Please check if the function has been called before!"));
	}

	if (ActorToDisplay)
	{
		ActorToDisplay->SetActorLocation(GetItemStageLocation());
		ActorToDisplay->SetActorRotation(GetItemStageRotation() + DeltaDisplayRot);
		DisplayedActor = ActorToDisplay;

		// 카메라 조정
		FVector ActorOrigin; // Unused
		FVector ActorExtent;
		ABaseItem* GameItem = Cast<ABaseItem>(DisplayedActor);
		if (GameItem)
		{
			ActorExtent = GameItem->GetEstimatedItemSize();
			AdjustCameraOnTarget(ActorExtent.Y, ActorExtent.Z);
		}
		else
		{
			DisplayedActor->GetActorBounds(true, ActorOrigin, ActorExtent);
			AdjustCameraOnTarget(ActorExtent.Z, ActorExtent.Y);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupDisplayActor - ActorToDisplay is nullptr!"));
	}
}

UTextureRenderTarget2D* AIconStageActor::CreateIconRenderTarget(int32 Width, int32 Height)
{
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->InitAutoFormat(Width, Height);
	return RenderTarget;
}

void AIconStageActor::SetTextureTargetAs(UTextureRenderTarget2D* Target)
{
	CaptureComponent->TextureTarget = Target;
}

void AIconStageActor::CaptureTarget()
{
	// 타깃 액터만 표시하도록 설정
	// 이 과정은 서버 및 클라 모두에게서 처리되어야 하며, 호출되기 전에 사전에 타깃 액터에 대한 정보가 레플리케이션 되어야 한다
	CaptureComponent->ShowOnlyActors = { DisplayedActor };

	// 수동 캡처를 처리하기 위한 과정이 다소 비직관적인데, 다음과 같다
	// 1. Auto Activate, Capture Every Frame false로 설정한 채 생성
	// 2. 캡처 시작 시 Capture Every Frame true로 설정 후 Activate() 호출
	// 3. CaptureSceneDeferred() 호출 - 캡처 처리
	// 4. 캡처가 완료되었다면 Deactivate(); 이때 bCaptureEveryFrame은 다시 false로 돌려놓는 게 아니라, true를 유지해야 한다
	CaptureComponent->bCaptureEveryFrame = true;
	CaptureComponent->Activate();
	CaptureComponent->CaptureSceneDeferred();
	CaptureComponent->Deactivate();
}

UTextureRenderTarget2D* AIconStageActor::GetRenderTarget() const
{
	if (CaptureComponent)
	{
		return CaptureComponent->TextureTarget;
	}
	return nullptr;
}

FVector AIconStageActor::GetItemStageLocation() const
{
	// TODO
	return GetActorLocation();
}

FRotator AIconStageActor::GetItemStageRotation() const
{
	// TODO
	return FRotator(0,0,0);
}

void AIconStageActor::AdjustCameraOnTarget(float TargetWidth, float TargetHeight)
{
	FVector DefaultRelativeLocaiton = CaptureComponent->GetRelativeLocation();

	// 아이템 크기에 따른 거리 조정
	const float CLOSEST_X_DIST = 50.f;
	const float FARTHEST_X_DIST = 120.f;

	// 배율 조정용 상수값
	// 가로세로 중 최장 길이가 10cm일때 아이템으로부터 19.5cm 떨어지도록 설정되어있음 (19.5/10)
	const float LEN_CONSTANT = 2.15f;

	float NewXDist = FMath::Clamp(TargetWidth * LEN_CONSTANT, CLOSEST_X_DIST, FARTHEST_X_DIST);
	CaptureComponent->SetRelativeLocation(FVector(NewXDist * -1/* 음의 방향으로 이동 */, DefaultRelativeLocaiton.Y, DefaultRelativeLocaiton.Z));
	return;
}

void AIconStageActor::BeginPlay()
{
	Super::BeginPlay();


}
