// Copyright (C) 2024 by Haguk Kim


#include "UserSpecificWidgetActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "TRMacros.h"

AUserSpecificWidgetActor::AUserSpecificWidgetActor()
{
	bReplicates = true;

	TObjectPtr<class UWidgetComponent> WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CoreWidgetComp"));
	check(WidgetComponent != nullptr);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetSimulatePhysics(false);
	WidgetComponent->SetEnableGravity(false);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

	// 기본적으로 지정된 호스트를 제외하면 보이지 않음
	WidgetComponent->SetVisibility(false);
	SetRootComponent(WidgetComponent);
}

void AUserSpecificWidgetActor::BeginPlay()
{
	Super::BeginPlay();

	// 서버-클라 공통로직
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("AUserSpecificWidgetActor::BeginPlay - Unexpected error!"));
		return;
	}
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(World, 0);
	if (LocalPC && TargetViewers.Contains(LocalPC))
	{
		Local_SetVisibility(true);
	}
}

void AUserSpecificWidgetActor::Local_SetVisibility(bool bVisibility)
{
	UWidgetComponent* WidgetComp = GetWidgetComp();
	if (WidgetComp)
	{
		WidgetComp->SetVisibility(bVisibility);
	}
}

void AUserSpecificWidgetActor::ClearData()
{
	TargetViewers.Empty();
}

void AUserSpecificWidgetActor::Server_AddTarget(APlayerController* Target)
{
	TargetViewers.Add(Target);
}

