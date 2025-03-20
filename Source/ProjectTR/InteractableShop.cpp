// Copyright (C) 2024 by Haguk Kim


#include "InteractableShop.h"
#include "GameCharacter.h"
#include "FPSCharacter.h"
#include "TRPlayerController.h"
#include "TRShop.h"

void AInteractableShop::BeginPlay()
{
	Super::BeginPlay();

	// 상점 로직 액터를 스폰해 트리거될 수 있도록 바인딩한다
	bool bShopLogicActorBound = false;
	UWorld* World = GetWorld();
	if (World && ShopActorClass)
	{
		ATRShop* ShopLogicActor = World->SpawnActor<ATRShop>(ShopActorClass, GetActorLocation() + FVector(0,0,100.0f), GetActorRotation());
		if (ShopLogicActor)
		{
			ShopLogicActorRef = ShopLogicActor;
			bShopLogicActorBound = true;
		}
	}

	if (!bShopLogicActorBound)
	{
		UE_LOG(LogTemp, Error, TEXT("AInteractableShop::BeginPlay - Failed to bind ATRShop actor!"));
	}
}

void AInteractableShop::OnMuzzleTriggered(AGameCharacter* TriggeredBy)
{
	if (!HasAuthority()) return;

	// 상점 인터랙션 시작
	if (ShopLogicActorRef)
	{
		// 인터랙션 전 필요 로직이 있을 경우 구현
		ShopLogicActorRef->TriggerThis();

		AFPSCharacter* ShoppingPlayer = Cast<AFPSCharacter>(TriggeredBy);
		if (ShoppingPlayer)
		{
			ATRPlayerController* ShoppingHost = Cast<ATRPlayerController>(ShoppingPlayer->GetController());
			if (ShoppingHost)
			{
				// 로컬 호스트에게 상점 UI를 띄우고 인터랙션 시작
				ShoppingHost->Local_StartShoppingRPC(ShopLogicActorRef);
			}
		}
	}
}
