// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "FPSCharacter.h"
#include "GameFramework/Actor.h"
#include "TRStructs.h"
#include "TRSoul.generated.h"

UCLASS()
class PROJECTTR_API ATRSoul : public ABaseItem
{
	GENERATED_BODY()
	
public:
	ATRSoul();

protected:
	virtual void BeginPlay() override;

protected:
	// NOTE: 현재로써 영혼 아이템의 ItemData를 사용한 복구는 서버에서만 처리되어도 문제가 없음
	virtual bool RestoreFromItemData(UItemData* Data) override;

public:
	// 이 영혼 아이템이 현재 새 플레이어 부활을 위해 사용될 수 있는지 여부
	// 필요한 정보가 누락되었거나,
	// 저장되어있는 플레이어 컨트롤러가 이미 AFPSCharacter 하위 인스턴스를 Possess중인 경우 false를 반환한다
	bool IsReadyToRespawnPlayer() const;

	/* Getters */
	TSubclassOf<AFPSCharacter> Server_GetCharacterClass() const { return Server_CharacterClass; }
	class ATRPlayerController* Server_GetController() const { return Server_Controller; }
	const FGameCharacterInstanceData& Server_GetInstanceData() const { return Server_InstanceData; }

	/* Setters */
	void Server_SetCharacterClass(TSubclassOf<class AFPSCharacter> Class) { Server_CharacterClass = Class; }
	void Server_SetController(class ATRPlayerController* Controller) { Server_Controller = Controller; }
	void Server_SetInstanceData(const FGameCharacterInstanceData Data) { Server_InstanceData = Data; }

protected:
	// 부활에 쓰이는 캐릭터 클래스
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class AFPSCharacter> Server_CharacterClass = nullptr;

	// 부활 대상 플레이어
	UPROPERTY(BlueprintReadOnly)
	class ATRPlayerController* Server_Controller = nullptr;

	// 부활 시 사용할 복구 데이터
	UPROPERTY(BlueprintReadOnly)
	FGameCharacterInstanceData Server_InstanceData;
};
