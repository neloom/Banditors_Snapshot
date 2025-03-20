// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "TRMacros.h"
#include "NiagaraComponent.h"
#include "TRPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTTR_API ATRPlayerController : public APlayerController
{
	GENERATED_BODY()
	
#pragma region /** Networking */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ATRPlayerController, CurrDungeonDepth);
	}
#pragma endregion

public:
	ATRPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 폰 Possession이 변경될 경우 서버와 클라 모두에게서 호출된다
	UFUNCTION()
	void Local_OnPawnPossessionChange(APawn* OldPosPawn, APawn* NewPosPawn);

#pragma region /** Widgets */
public:
	// 이 호스트에 대해 로컬 위젯을 생성한다
	// 만약 특정 폰에 바운딩 되어있다면 폰을 인자로 전달한다
	UUserWidget* Local_CreateWidget(TSubclassOf<UUserWidget> WidgetClass, APawn* BoundedTo = nullptr);

	// 위젯을 로컬 뷰포트에 추가한다
	void Local_DisplayWidget(class UUserWidget* Widget, int32 ZOrder = WZO_DEFAULT);

	// 위젯을 화면 상에서 숨긴다(collapse); 이 함수는 뷰포트로부터 위젯을 제거하지는 않는다
	void Local_CollapseWidget(class UUserWidget* Widget);

	// 위젯에 포커싱을 맞추고 인풋을 해당 위젯에 한정되도록 전환한다
	void Local_FocusWidget(class UUserWidget* Widget, bool bShowCursor, bool bFlushKey);

	// 포커스를 게임으로 전환한다
	void Local_FocusGame(bool bShowCursor, bool bFlushKey);

	// 주어진 위젯의 레퍼런스를 모두 제거한다
	void Local_DerefWidget(class UUserWidget* Widget);

	// 해당 캐릭터에 바운딩된 위젯의 레퍼런스를 모두 제거한다
	void Local_DerefPawnBoundedWidgets(class APawn* BoundPawn);

	// 이 호스트 컨트롤러에 바운딩된 위젯의 레퍼런스를 모두 제거한다
	void Local_DerefHostBoundedWidgets();

protected:
	// 어떤 특정 폰의 존재와 위젯의 생명주기가 바운딩 되어있을 경우 이 곳에 등록된다
	// i.e. 인벤토리 위젯은 인벤토리 소유 액터가 사망하면 사라져야 한다
	TMap<APawn*, TSet<UUserWidget*>> PawnBoundWidgets;

	// 호스트 자체와 위젯이 바운딩 되어있을 경우 이 곳에 등록된다
	TArray<UUserWidget*> HostBoundWidgets;

public:
	// 이 컨트롤러의 소유자에게 해당하는 텍스트를 표기한다
	UFUNCTION(Client, Reliable)
	void Local_AlertTextRPC(const FString& Text, float Duration);

	// 이 컨트롤러의 소유자에게 상점 UI를 표시하고 인터랙션을 시작한다
	UFUNCTION(Client, Reliable)
	void Local_StartShoppingRPC(class ATRShop* ShopLogicActor);

public:
	// Alert를 위해 사용할 수 있는 텍스트 표기 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	TSubclassOf<class UTextAlertWidget> TextAlertWidgetClass;

	// 인벤토리 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	TSubclassOf<class UUserWidget> InvWidgetClass;

	// 상점 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	TSubclassOf<class UUserWidget> ShopWidgetClass;

	// HUD 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	TSubclassOf<class UUserWidget> HUDWidgetClass;
#pragma endregion

#pragma region /** Gameplay */
public:
	// 이 캐릭터가 속한 던전 깊이를 설정한다
	void Server_SetCurrDungeonDepth(int32 Depth);

	// 이 캐릭터가 속한 던전 깊이를 반환한다
	// 레플리케이션 지연 등으로 인해 클라이언트에서는 정확하지 않은 값이 반환될 수 있다
	UFUNCTION(BlueprintCallable)
	int32 Local_GetCurrDungeonDepth();

protected:
	UFUNCTION()
	void OnRep_CurrDungeonDepth();
	void Local_OnCurrDungeonDepthUpdated();

protected:
	// 현재 속한 던전 층계
	// NOTE: 서버에서 Authoritative한 값에 접근하기 위해서는 이 값 대신 게임모드의 값을 사용할 것
	UPROPERTY(ReplicatedUsing = OnRep_CurrDungeonDepth)
	int32 CurrDungeonDepth = 0;
#pragma endregion

#pragma region /** Niagara Pooling */
// 언리얼의 기본 나이아가라 풀링은 풀에 Free 슬롯이 없을때 새 자원을 요청할 경우, 새 인스턴스를 생성하는 방식으로 처리함
// 그러나 고속 연사를 처리할 때 이 방식은 메모리 제한 없이 계속 확보해나가기 때문에 지나치게 부하가 커질 위험이 있음
// 따라서 상한선을 설정하기 위해 현재 존재하는 인스턴스들을 추적하는 방식을 사용함
// 이 방식은 모든 나이아가라에 대해 다 해줄 필요는 없으며, 짧은 시간동안 다수의 인스턴스가 생성될 수 있는 (주로 총기 격발 관련) VFX에 대해서만 사용하면 됨
// 사용 시에는 반드시 지정된 형식을 따라 풀에 등록 및 반환해야 함
public:
	// VFX 종류별로 현재 존재하는 컴포넌트 인스턴스들과 각각의 개수 추적
	// NOTE: 나이아가라가 도중에 파괴될 수 있기 때문에 WeakPtr 사용이 필수적이다
	TQueue<TWeakObjectPtr<UNiagaraComponent>> HitVFXPendingRelease;
	int32 CurrHitVFXPendingReleaseCnt = 0;
	TQueue<TWeakObjectPtr<UNiagaraComponent>> TracerVFXPendingRelease;
	int32 CurrTracerVFXPendingReleaseCnt = 0;
	TQueue<TWeakObjectPtr<UNiagaraComponent>> ExplVFXPendingRelease;
	int32 CurrExplVFXPendingReleaseCnt = 0;

	// 동시에 Release 대기 상태로 있을 수 있는 (=동시에 존재 가능한) 인스턴스들의 최대 개수
	// NOTE: 이 값은 나이아가라 에디터에서 개별적으로 설정할 수 있는 PoolSize 값을 대체한다
	// 정확히 말하면 기능을 완전히 대체하는 것은 아니지만, 커스텀 풀링을 사용할 경우 Used 항목이 사실상 Free 항목의 역할을 겸하기 때문에
	// 이 값이 실질적으로 최대 풀의 크기와 더 가깝다고 이해하면 된다
	// 세부적인 수치를 확인하려면 FX.DumpNCPoolInfo 콘솔 명령어 사용
	int32 MaxCoexistingHitVFXCnt = TR_NIAGARA_MAX_HIT_VFX_CNT;
	int32 MaxCoexistingTracerVFXCnt = TR_NIAGARA_MAX_TRACER_VFX_CNT;
	int32 MaxCoexistingExplVFXCnt = TR_NIAGARA_MAX_EXPLOSION_VFX_CNT;
#pragma endregion
};
