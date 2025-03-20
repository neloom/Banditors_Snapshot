// Copyright (C) 2024 by Haguk Kim


#include "SlotComponent.h"

// Sets default values for this component's properties
USlotComponent::USlotComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


void USlotComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void USlotComponent::InitGridSize()
{
	// Rows, Columns 재정의
	Rows = 1;
	Columns = SlotCount;
}

FInvObjSize USlotComponent::GetGridDimensions(UInvObject* InvObj) const
{
	return { 1, 1 };
}

void USlotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USlotComponent::SwitchSlotTo(int32 SlotIdx)
{
	CurrentSlot = SlotIdx;
}

UInvObject* USlotComponent::GetInvObjOfSlot(int32 SlotIdx) const
{
	return GetInvObjAtXY(SlotIdx, 0);
}

UInvObject* USlotComponent::GetInvObjOfCurrSlot() const
{
	return GetInvObjOfSlot(CurrentSlot);
}

bool USlotComponent::TryAddInvObjToSlot(UInvObject* InvObj, int32 SlotIdx)
{
	return TryAddInvObjectAt(InvObj, SlotIdx, 0);
}

