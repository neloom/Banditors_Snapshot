// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ReplicatedObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API UReplicatedObject : public UObject
{
	GENERATED_BODY()

protected:
	// Outer의 World 사용
	virtual UWorld* GetWorld() const override
	{
		if (const UObject* MyOuter = GetOuter())
		{
			return MyOuter->GetWorld();
		}
		return nullptr;
	}

	UFUNCTION(BlueprintPure)
	AActor* GetOwningActor() const
	{
		return GetTypedOuter<AActor>();
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		// 블루프린트 프로퍼티 레플리케이션
		if (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
		{
			BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
		}
	}

	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}

	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override
	{
		check(GetOuter() != nullptr);
		return GetOuter()->GetFunctionCallspace(Function, Stack);
	}

	// Owner액터의 Netdriver를 사용해 RemoteFunction 호출
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override
	{
		check(!HasAnyFlags(RF_ClassDefaultObject));
		AActor* Owner = GetOwningActor();
		UNetDriver* NetDriver = Owner->GetNetDriver();
		if (NetDriver)
		{
			NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
			return true;
		}
		return false;
	}
};
