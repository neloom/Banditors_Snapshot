// Copyright (C) 2025 by Haguk Kim


#include "LocalDamageNumber.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "TRMacros.h"
#include "Components/WidgetComponent.h"

int32 ALocalDamageNumber::CurrInstCount = 0;

ALocalDamageNumber::ALocalDamageNumber()
{
	PrimaryActorTick.bCanEverTick = false;

	TObjectPtr<class UWidgetComponent> WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CoreWidgetComp"));
	check(WidgetComponent != nullptr);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetSimulatePhysics(false);
	WidgetComponent->SetEnableGravity(false);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	SetRootComponent(WidgetComponent);
}

void ALocalDamageNumber::BeginPlay()
{
	Super::BeginPlay();

	ALocalDamageNumber::CurrInstCount++;
}

void ALocalDamageNumber::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ALocalDamageNumber::CurrInstCount--;
}

void ALocalDamageNumber::SetWidgetClassAs(TSubclassOf<UDamageNumberWidget> WidgetClass)
{
	if (WidgetClass && WidgetClass != DamageNumberWidgetClass /* 기존에 이미 설정된 위젯이 있고 클래스가 동일한 경우 새로 생성할 필요 없음 */)
	{
		UWidgetComponent* WidgetComp = GetWidgetComp();
		if (!WidgetComp)
		{
			UE_LOG(LogTemp, Error, TEXT("ALocalDamageNumber::SetWidgetClassAs - WidgetComp is invalid! Something went wrong!!"));
			return;
		}

		// 기존에 생성한 위젯이 있을 경우 제거
		if (WidgetComp->GetWidget())
		{
			WidgetComp->GetWidget()->RemoveFromParent();
		}

		UDamageNumberWidget* Widget = CreateWidget<UDamageNumberWidget>(GetWorld(), WidgetClass);
		if (Widget)
		{
			WidgetComp->SetWidget(Widget);

			if (Widget->DmgNumberAnim)
			{
				EstimatedDisplayTime = Widget->DmgNumberAnim->GetEndTime();
			}
		}
	}
}

void ALocalDamageNumber::StartDisplay(int32 Damage)
{
	UWidgetComponent* WidgetComp = GetWidgetComp();
	if (WidgetComp)
	{
		WidgetComp->Activate();
		UDamageNumberWidget* Widget = Cast<UDamageNumberWidget>(WidgetComp->GetWidget());
		if (Widget)
		{
			Widget->SetDamage(Damage);
			Widget->SetVisibility(ESlateVisibility::Visible);
			Widget->Local_StartDisplaying();

			// 애니메이션 종료 후 반환
			// NOTE: 이는 실제 애니메이션의 종료 시점과 다를 수 있다; 화면 밖으로 벗어난 애니메이션의 경우 재생을 중단하기 때문인데,
			// 따라서 Widget에 내장된 Animation finish 델리게이트 대신, 수동으로 애니메이션 재생 시간만큼 타이머를 사용하는 것으로
			// 설사 위젯이 시야 밖으로 나가 컬링되더라도 제때 반환되도록 설정한다
			if (GetWorld()) GetWorld()->GetTimerManager().SetTimer(DisplayTimer, this, &ALocalDamageNumber::OnUIAnimEnd, EstimatedDisplayTime, false);
		}
	}
}

void ALocalDamageNumber::StopDisplay(bool bClearWidgetData)
{
	UWidgetComponent* WidgetComp = GetWidgetComp();
	if (WidgetComp)
	{
		WidgetComp->Deactivate();
		UDamageNumberWidget* Widget = Cast<UDamageNumberWidget>(WidgetComp->GetWidget());
		if (Widget)
		{
			if (bClearWidgetData) Widget->ClearWidgetData();
			Widget->SetVisibility(ESlateVisibility::Collapsed);

			if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(DisplayTimer);
		}
	}
}

void ALocalDamageNumber::OnUIAnimEnd()
{
	StopDisplay(true);

	if (ReturnPool && ALocalDamageNumber::CurrInstCount <= ALocalDamageNumber::MaxInstCount)
	{
		TR_PRINT_FSTRING("OnUIAnimEnd -  Enqueued; Q:%d", ALocalDamageNumber::CurrInstCount);
		ReturnPool->Enqueue(this);
	}
	else
	{
		TR_PRINT("OnUIAnimEnd -  Destroyed");
		UE_LOG(LogTemp, Error, TEXT("ALocalDamageNumber::OnUIAnimEnd - Either actor is invalid so it cannot return it back to pool, or the instance count exceeded the limit. Destroying the object."));
		Destroy();
	}
}
