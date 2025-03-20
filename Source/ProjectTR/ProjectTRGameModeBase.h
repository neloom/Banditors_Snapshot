// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "TRStructs.h"
#include "TREnums.h"
#include "TRUtils.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"
#include "DamageNumber.h"
#include "SpawnPoint.h"
#include "ProjectTRGameModeBase.generated.h"

/**
 * 게임모드의 모든 함수는 Authoritative 호스트에서만 실행됨이 보장된다
 */
UCLASS()
class PROJECTTR_API AProjectTRGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
	AProjectTRGameModeBase();

protected:
	virtual void StartPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	// 게임 인스턴스 Getter
	class UTRGameInstance* GetTRGameInstance();

	// 플레이어 이름을 업데이트
	// 이미 동일한 정보로 업데이트 한 내역이 있다면 불필요한 네트워크 부하를 막기 위해 아무 것도 처리하지 않는다
	void UpdatePlayerNames();

protected:
	// 경로 불문하고 현재 스폰된 봇들의 수; 보스에는 해당되지 않는다
	// 델리게이트를 사용해 파괴시 감소하는 형태로 관리된다
	int32 CurrSpawnedCount = 0;

#pragma region /** Spawn */
// NOTE: 인자로 전달되는 FActorSpawnParameters는 자동으로 적용되지 않으며, 적용이 필요한 정보를 묶어 전달하기 위한 매개체로 사용된다
// 따라서 CollisionResponse, Instigator 등을 지정하기 위해서는 Params 내 변수들을 사용해 직접 값을 변경해주어야 한다
public:
	// 아이템 액터 생성을 위해 사용한다
	class ABaseItem* SpawnItem(TSubclassOf<class ABaseItem> ItemClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params);

	// 게임플레이에 영향을 주지 않는, 장식용 아이템을 생성하기 위해 사용한다
	class ABaseItem* SpawnDecorativeItem(class UInvObject* InvObj, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params);

	// 주어진 InvObject, ItemData를 기반으로 아이템 액터를 재생성 하기 위해 사용한다
	class ABaseItem* RespawnItem(TSubclassOf<class ABaseItem> ItemClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params, class UInvObject* InvObject, class UItemData* ItemData);

	// 총기 랜덤 생성을 위해 사용한다
	// 티어에 0 이하의 값이 주어질 경우 각 파츠들의 티어 분포는 완전히 랜덤한 값을 가지게 된다
	class AGunItem* SpawnRandomizedGunItem(UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params, int32 TokenTier);

	// 관전자 폰 생성을 위해 사용한다
	class ATRSpectatorPawn* SpawnSpectator(UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params);

	// 봇 생성을 위해 사용한다
	// 컨트롤러는 별도로 할당해주어야 한다
	class ABotCharacter* SpawnBot(TSubclassOf<class ABotCharacter> BotClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params);

	// 지형지물 및 기타 던전 내 액터 생성을 위해 사용한다
	class ADungeonActor* SpawnDungeonActor(TSubclassOf<class ADungeonActor> ActorClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params);

	// 주어진 티어에 해당하는 토큰 아이템을 생성하기 위해 사용한다
	class ATRToken* SpawnToken(TSubclassOf<class ATRToken> TokenClass, UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params, int32 TokenTier);

	// 아이콘 생성에 사용될 액터 및 아이콘에 해당하는 임시 아이템을 생성한다
	// 여기서 생성된 임시 아이템은 오직 시각적인 용도로만 사용되어야 하며, 반드시 게임플레이에 영향을 주지 못하도록 차단해야 한다
	// 아이템 액터가 월드에 존재하는 것만으로 게임에 영향을 주는 경우 주의가 필요하다
	class AIconStageActor* SpawnIconStage(UWorld* World, FVector Location, FRotator Rotation, FActorSpawnParameters Params);

	// 폭발 액터를 생성한다
	// 만약 폭발의 로직을 수정해야 할 경우 ExplosionInfo를 넘길때 bOverrideExplInfo를 true로 설정해 주어야 한다 
	class ATRExplosion* SpawnExplosion(TSubclassOf<class ATRExplosion> ExplClass, UWorld* World, FVector Location, FRotator Rotation, FVector CollisionNormal, FActorSpawnParameters Params, const FExplosionInfo& NewExplInfo, bool bOverrideExplInfo = true);

protected:
	// 총기 뼈대가 될 클래스
	// 총기별 세부 사항은 클래스의 종류가 아닌, 파츠 구성에 의해 결정된다
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class AGunItem> DefaultGunItemClass = nullptr;

public:
	// 몬스터 생성 시 할당해줄 컨트롤러 오브젝트 풀
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UAIControllerPool> AIPool = nullptr;
#pragma endregion

#pragma region /** Token generation */
public:
	// 플레이어가 주어진 레벨로 레벨업 했을 때 리워드로 제공되는 토큰의 티어를 결정하기 위해 사용한다
	int32 RandomizeTokenTier(int32 PlayerLevel);

	// 주어진 토큰 정보를 통해 총기 구성 파츠들의 개별 티어들을 결정한다
	FGunPartTierData RandomizePartsTier(int32 TokenTier);
#pragma endregion

#pragma region /** Gun generation */
protected:
	// 총기 컴포넌트를 랜덤으로 선택해 오브젝트를 생성한다
	void RandomizeGunParts(const FGunPartTierData& TierData, class AGunItem* GunItem, class UGunPartComponent** ReceiverComp, class UGunPartComponent** BarrelComp, class UGunPartComponent** GripComp, class UGunPartComponent** MagazineComp, class UGunPartComponent** MuzzleComp, class UGunPartComponent** SightComp, class UGunPartComponent** StockComp);

	// 특정 타입과 티어에 대해 파츠를 찾아 생성한다
	// bDowngradeTierIfEmpty가 true일 경우,
	// 해당 티어에 속하는 파츠가 없을 경우 티어를 하나씩 낮추며 파츠를 찾는다
	UGunPartComponent* SelectAndCreatePart(class AGunItem* GunItem, const EGunPartType PartType, const int32 PartTier, bool bDowngradeTierIfEmpty = false);

	// 선택 가능한 파츠 목록을 초기화
	void InitPartsList();

	// 주어진 정보를 가지는 파츠에 대해 사용할 적절한 TArray의 레퍼런스 페어를 반환한다
	TPair<TArray<UClass*>*, TArray<float>*> SelectPartsList(const EGunPartType PartType, const int PartTier);
	
	// 파츠 정보를 클래스에 따라 적절한 리스트에 추가한다
	// T는 UGunPartComponent 혹은 그 자손으로, 잘못된 인자가 주어질 경우 컴파일 에러가 발생하므로 별도의 체킹은 필요하지 않다
	template<typename T>
	void AddPartToList()
	{
		UClass* PartClass = T::StaticClass();
		const int PartTier = T::GetTier();
		const float PartSpawnRate = T::GetSpawnRate();
		const EGunPartType PartType = T::GetPartType();

		if (!IsValid(PartClass))
		{
			UE_LOG(LogTemp, Error, TEXT("AddPartToList - Unknown error!"));
			return;
		}
		if (PartType == EGunPartType::EGT_Undefined)
		{
			UE_LOG(LogTemp, Error, TEXT("AddPartToList - GunPart %s has invalid part type. Please redefine the static function correctly."), *PartClass->GetName());
			return;
		}
		
		TPair<TArray<UClass*>*, TArray<float>*> ListPair;
		ListPair = SelectPartsList(PartType, PartTier);
		ListPair.Get<0>()->Add(PartClass);
		ListPair.Get<1>()->Add(PartSpawnRate);
	}

protected:
	// Static class들의 목록과 각각의 선택 확률(weight)들을 저장
	TArray<UClass*> ReceiverClasses_T1;
	TArray<float> ReceiverWeights_T1;
	TArray<UClass*> ReceiverClasses_T2;
	TArray<float> ReceiverWeights_T2;
	TArray<UClass*> ReceiverClasses_T3;
	TArray<float> ReceiverWeights_T3;

	TArray<UClass*> BarrelClasses_T1;
	TArray<float> BarrelWeights_T1;
	TArray<UClass*> BarrelClasses_T2;
	TArray<float> BarrelWeights_T2;
	TArray<UClass*> BarrelClasses_T3;
	TArray<float> BarrelWeights_T3;

	TArray<UClass*> GripClasses_T1;
	TArray<float> GripWeights_T1;
	TArray<UClass*> GripClasses_T2;
	TArray<float> GripWeights_T2;
	TArray<UClass*> GripClasses_T3;
	TArray<float> GripWeights_T3;

	TArray<UClass*> MagazineClasses_T1;
	TArray<float> MagazineWeights_T1;
	TArray<UClass*> MagazineClasses_T2;
	TArray<float> MagazineWeights_T2;
	TArray<UClass*> MagazineClasses_T3;
	TArray<float> MagazineWeights_T3;

	TArray<UClass*> MuzzleClasses_T1;
	TArray<float> MuzzleWeights_T1;
	TArray<UClass*> MuzzleClasses_T2;
	TArray<float> MuzzleWeights_T2;
	TArray<UClass*> MuzzleClasses_T3;
	TArray<float> MuzzleWeights_T3;

	TArray<UClass*> SightClasses_T1;
	TArray<float> SightWeights_T1;
	TArray<UClass*> SightClasses_T2;
	TArray<float> SightWeights_T2;
	TArray<UClass*> SightClasses_T3;
	TArray<float> SightWeights_T3;

	TArray<UClass*> StockClasses_T1;
	TArray<float> StockWeights_T1;
	TArray<UClass*> StockClasses_T2;
	TArray<float> StockWeights_T2;
	TArray<UClass*> StockClasses_T3;
	TArray<float> StockWeights_T3;
#pragma endregion

#pragma region /** Level transition */
public:
	// 레벨 변경 시 호출
	// 깊이 변경이 없을 경우 NewDepth에 음수 전달
	void ChangeGameLevel(FString LevelURL, int32 NewDepth = -1);

	// 레벨 생성 직후 던전 깊이를 설정하기 위해 사용
	void InitDungeonDepth(int32 NewDepth);

	// 모든 플레이어들에게 던전 깊이를 업데이트하기 위해 사용
	// 새 호스트의 접속 발생 시 호출
	void UpdatePlayersDungeonDepth();

	// 층의 깊이가 주어졌을 때 그 층에 해당하는 레벨의 이름을 반환한다
	FString GetLevelNameOfDepth(int32 TargetDepth);

	// Getters
	int32 GetCurrentDungeonDepth() const { return DungeonDepth; }

protected:
	// 현재 레벨의 던전 깊이
	int32 DungeonDepth = 0;

	// 각 층별 타깃 던전 레벨의 이름
	// 만약 특정 깊이에 대해 값이 존재하지 않을 경우 기본 던전 맵을 사용한다
	UPROPERTY(EditDefaultsOnly)
	TMap<int32, FString> DungeonLevels;
#pragma endregion

#pragma region /** Players */
public:
	// 접속한 모든 플레이어들의 목록
	TArray<class ATRPlayerController*> GetPlayersConnected();

	// 현재 유효한(게임플레이에 영향을 줄 수 있는) 모든 플레이어 캐릭터들의 목록
	// FPSCharacter가 아닌 다른 폰을 possess 중인 경우나 bIsOut이 true인 경우 반환하지 않는다 (관전 상태 등)
	// 사망한 캐릭터를 무시하기 위해 bIgnoreDead를 true로 설정할 수 있다, 
	// 단 일반적으로 사망한 캐릭터를 계속 possess하는 경우는 없기 때문에 큰 영향은 없다
	TArray<class AFPSCharacter*> GetInGamePlayerCharacters(bool bIgnoreDead);

	// 특정 레벨의 주어진 위치에 주어진 클래스 인스턴스를 새로 스폰하고 컨트롤러를 Possess 시킨다
	// 클래스 타입이 주어지지 않은 경우 이 게임모드에 할당된 Default Pawn Class를 사용한다
	// 인스턴스 정보가 주어진 경우 해당 정보를 사용해 스폰된 인스턴스를 편집한다
	// NOTE: 컨트롤러가 현재 Possess 중인 Pawn이 있을 경우 이를 Destroy하므로, 기존 폰을 살려야 할 경우 먼저 UnPossess 시킨 후 호출해야 한다
	void RespawnPlayer(class ATRPlayerController* Controller, FTransform RespawnTransform, TSubclassOf<class AFPSCharacter> CharacterClass, FGameCharacterInstanceData InstanceData, bool bRegenHealthAndLife);
#pragma endregion

#pragma region /** Game flow */
/* Game Over */
public:
	// 게임오버에 영향을 줄 만한 상태 변화 발생 시 호출한다
	void UpdateGameOverStatus();

	// 게임 오버 시 처리 로직
	void OnGameOver();

	/* Getters */
	// 게임 오버되었는지 확인한다
	bool IsGameOver() { return bIsGameOver; }

/* Game Clear */
public:
	// 게임 클리어에 영향을 줄 만한 상태 변화 발생 시 호출한다
	void UpdateGameClearStatus();

	// 게임 클리어 시 처리 로직
	void OnGameClear();

	/* Getters */
	// 게임 클리어 되었는지 확인한다
	bool IsGameClear() { return bIsGameClear; }
protected:
	// 현재 게임 오버 여부
	bool bIsGameOver = false;

	// 현재 게임 클리어 여부
	bool bIsGameClear = false;

/* Dungeon state */
public:
	// 현재 레드모드인지 여부
	bool IsDungeonRedMode() { return GetCurrentEnemyGenState() == EEnemyGenerationState::EGS_RED; }
#pragma endregion

#pragma region /** Dungeon Timer */
protected:
	// 던전 입장 후 경과 시간 (sec) (서버 기준)
	float DungeonTime = 0.0f;

	// 던전 현재 층계 탈출까지 주어지는 총 시간 (sec)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DungeonTotalTimeGiven = 10.0f;

	// 시간 경과 여부
	bool bIsTimeOver = false;

protected:
	// 던전 타이머 갱신
	void UpdateDungeonTime(float DeltaTime);

	// 탈출 시간 경과 시 1회 호출
	void OnTimeOver();

	// 즉각 레드모드에 진입한다
	void EnterRedMode();

	// 레드모드 진입 시 로직
	void OnEnterRedMode();

public:
	// 밀리초 제거 후 반환
	FORCEINLINE int32 GetDungeonTimeSecond() { return FMath::RoundToInt(DungeonTime); }

	// 탈출까지 남은 시간
	// 시간이 경과한 경우 0을 반환한다
	FORCEINLINE int32 GetSecondsLeft() { return FMath::Max<int32>(0, FMath::RoundToInt(DungeonTotalTimeGiven - DungeonTime)); }
#pragma endregion

#pragma region /** Dungeon Generation */
public:
	// 현재 맵에 Dungeon Generator 인스턴스가 있을 경우 StartPlay에서 자동적으로 바인딩된다
	TObjectPtr<class ATRDungeonGenerator> DungeonGenerator = nullptr;

	// 던전 생성 완료 후 전달받아야 하는 모든 정보를 건내받아, 던전 로직을 처리할 준비가 되었는지 여부
	bool bDungeonGenerationDataReceived = false;

	// Intensity 최대, 최솟값
	const float MIN_INTENSITY = 0.0f;
	const float MAX_INTENSITY = 1.0f;

	// 스폰 범위 오프셋
	// 이 값을 기준으로 디스폰, 스폰 룸들의 범위가 결정된다
	// 현재 Occulded된 방들 + 해당 값의 범위가 유효 스폰 구역이다
	UPROPERTY(EditDefaultsOnly)
	uint32 SpawnDepthOffset = 1;

	// 스폰 주기 최소, 최대값
	// Intensity가 최대, 최소치일때 사용하는 스폰 주기이다 (Intensity에 반비례)
	UPROPERTY(EditDefaultsOnly)
	float MinSpawnWaveCycle = 0.33f;

	UPROPERTY(EditDefaultsOnly)
	float MaxSpawnWaveCycle = 3.0f;

	// 동시에 공존하는 액터 수의 목표값
	UPROPERTY(EditDefaultsOnly)
	int32 MinEnemyCount = 10;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxEnemyCount = 30;

	// 활성화 상태인 액터의 비율
	// 해당 비율 이하로 내려갈 경우 오쿨루전 밖 액터이더라도 AI가 활성화 될 수 있다
	// 이러한 장치를 사용하는 이유는 모든 봇이 비활성화 된 상태에서 플레이어가 한 곳에 가만히 머무를 때
	// 아무런 변화가 발생하지 않아 무한히 대기가 가능해지는 일을 방지하기 위함이다
	UPROPERTY(EditDefaultsOnly)
	float MinActiveBotRatio = 0.25f;

	// 디스폰 웨이브의 최소 대기 시간
	// 디스폰은 주기가 지났고 스폰 웨이브가 실행되지 않는 틱에 실행된다
	const float DESPAWN_WAVE_DELAY = 1.0f;

protected:
	// [0, 1] 범위
	// 현재 게임 상황이 얼마나 극적인지를 설정
	// 가급적 Setter와 Getter를 사용할 것
	float Intensity = MIN_INTENSITY;

	// 목표 Intensity 값
	// 가급적 Setter와 Getter를 사용할 것
	float TargetIntensity = MIN_INTENSITY;

	// 현재 사용할 인텐시티 초당 증감폭
	float DeltaIntensityPerSec = 0.0f;

	// 현재 적 생성 스테이트
	EEnemyGenerationState EnemyGenState = EEnemyGenerationState::EGS_IDLE;

	// 현재 적 생성 스테이트의 유지 시간 (second)
	float CurrEnemyGenStateTime = 0.0f;

	// 현재 적 생성 스테이트의 목표 지속 시간
	// 음수의 경우 목표 지속 시간이 없음을 의미함
	// 현재 유지 시간이 최소 목표 지속 시간 이상이어야 다른 스테이트로의 전환이 가능하다
	float CurrEnemyGenStateTargetTime = -1.0f;

	// 마지막 스폰 웨이브 이후로 경과한 시간
	float TimeSinceLastSpawnWave = 0.0f;

	// 마지막 디스폰 웨이브 이후로 경과한 시간
	float TimeSinceLastDespawnWave = 0.0f;

	// 스폰 사이클에 의해 스폰된 적들의 목록
	// NOTE: 이 세트 내에는 이미 Destroy된 액터 포인터가 있을 수 있다
	// 따라서 액터의 숫자를 구하기 위해서는 .Num() 대신 CurrSpawnedCount 변수를 사용할 것
	TSet<class ABotCharacter*> SpawnedBots;

protected:
	// 던전 생성 시스템에 의해 스폰 가능한 레귤러(보스가 아닌) 봇들의 정보
	UPROPERTY(EditDefaultsOnly)
	TArray<FSpawnableMonsterData> SpawnableRegularBotClasses;

public:
	// 게임 로직단에서 적 생성을 관리하기 위해 사용할 수 있는 유일한 함수로, 목표 Intensity와 그 Intensity까지 도달하는데 걸려야 하는 시간을 설정하고
	// 그 후 스테이트를 리프레시한다
	// 이 함수는 기본 적 생성 사이클을 게임 로직 단에서 인터럽트하기 위해 사용할 수 있다
	// ReachTime이 0 이하인 경우 Intensity가 설정한 IntensityGoal로 즉각 변경된다
	void ChangeEnemyGeneration(float IntensityGoal, float ReachTime = -1.0f);

	// 생성 웨이브에 의해 생성된 봇이 더이상 로직에 영향을 주지 않게 되는 모든 경우에 호출해야 한다
	// 가장 대표적인 경우는 디스폰될 때, 혹은 사망해 래그돌화 될때가 있다
	// 그냥 편리하게 파괴 시점에 바인딩할 수도 있지만 그러는 대신 굳이 분기를 나누는 이유는, 
	// 래그돌 지속시간으로 인해 액터 스폰 수가 영향받는 일을 방지하기 위함이다 (래그돌은 파괴되지 않은 액터이기 때문)
	UFUNCTION()
	void OnBotInvalidated(ACharacter* RemoveCharacter);

	// Getters
	const EEnemyGenerationState GetCurrentEnemyGenState() { return EnemyGenState; }
	float GetIntensity() { return Intensity; }
	float GetTargetIntensity() { return TargetIntensity; }
	float GetDeltaIntensityPerSec() { return DeltaIntensityPerSec; }

protected:
	// Setter는 public하게 풀어놓지 않는다
	void SetIntensity(float Value);
	void SetTargetIntensity(float Value);
	void SetDeltaIntensityPerSec(float Value);

	// 바인딩 로직
	void FindAndBindDungeonGenerator();

	// 던전에서 매 틱마다 처리해야 할 로직 수행
	void DungeonTick(float DeltaTime);

	// Intensity 재계산
	void UpdateIntensity(float DeltaTime);

	// state machine 업데이트
	// 현재 Intensity에 따라 스테이트 정보만 변경해주는 역할을 한다
	// NOTE: 레드모드에 진입한 경우 해당 함수는 Intensity와 무관하게 어떠한 정보도 변경하지 않는다
	void RefreshEnemyGenSM();

	// 현재 스테이트가 종료된 경우 호출된다
	// 기본 사이클 상에서 현재 스테이트에서 다음에 와야할 스테이트를 구하고,
	// 그렇게 넘어가기 위해 변경해야 하는 값들을 변경한다
	void ChangeToNextState();

	// 적 생성 스테이트를 변경
	void SetEnemyGenState(EEnemyGenerationState NewState);

	// 현재 적 생성 스테이트 및 게임플레이 상황을 고려해 필요 시 적들을 생성 혹은 삭제
	void ManageEnemies(float DeltaTime);

	// 현재 적 생성 목표치 반환
	int32 GetCurrDesiredEnemyCount() const;

	// 현재 스폰웨이브 주기 소요시간 계산 후 반환
	FORCEINLINE float GetCurrSpawnWaveCycle() const;

	// 주어진 조건을 고려해 스폰 가능한 랜덤한 봇을 선택해 반환한다
	TSubclassOf<class ABotCharacter> GetRandomSpawnMonsterClass(int32 SpawnDepth);

	// 주어진 봇을 디스폰한다
	// 디스폰 성공 여부를 반환한다
	bool DespawnBot(class ABotCharacter* Bot);

/* Default cycle values */
protected:
	UPROPERTY(EditDefaultsOnly)
	FVector2D RelaxTimeRange = FVector2D(30.0f, 40.0f);

	UPROPERTY(EditDefaultsOnly)
	FVector2D RiseTimeRange = FVector2D(40.0f, 80.0f);

	UPROPERTY(EditDefaultsOnly)
	FVector2D PeakTimeRange = FVector2D(3.0f, 5.0f);

	UPROPERTY(EditDefaultsOnly)
	FVector2D FallTimeRange = FVector2D(5.0f, 10.0f);
#pragma endregion

#pragma region /** Item Icon Generation */
public:
	// 주어진 InvObject를 대상으로 아이콘 정보를 갱신하기 위한 로직의 진입지점
	void UpdateIconOf(class UInvObject* InvObj);

	// 생성할 IconStageActor의 Location을 새로 발급한다
	// 기존 생성된 다른 액터들과 겹치지 않도록 조정하며, 게임플레이에 영향이 없는 먼 위치에 생성한다
	// 가로로 긴 행의 형태로 생성한다
	FVector IssueNewSpawnLocation();

public:
	// 아이콘 생성 시 사용할 기본 스테이지 액터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AIconStageActor> IconStageClass = nullptr;

protected:
	// 최초 생성 위치
	UPROPERTY(EditDefaultsOnly)
	FVector BaseSpawnLocation = FVector(-65535.0f, -65535.0f, 65535.0f);

	UPROPERTY(EditDefaultsOnly)
	FRotator BaseSpawnRotation = FRotator();

	// 매 생성마다 더해줄 가로 델타
	UPROPERTY(EditDefaultsOnly)
	FVector SpawnLocatonHorizontalDelta = FVector(0.0f, 1200.0f, 0.0f);

	// 매 생성마다 더해줄 가로 델타
	UPROPERTY(EditDefaultsOnly)
	FVector SpawnLocatonVerticalDelta = FVector(2000.0f, 0.0f, 0.0f);

private:
	// 현재까지 발급된 아이콘 액터 트랜스폼 수
	// 실제 맵에 존재하는 아이콘 액터의 수와 차이가 있을 수 있다
	int32 IconTransformIssueCount = 0;

	// 행 한줄 당 들어갈 아이콘 개수
	int32 IconsPerRow = 100;
#pragma endregion

#pragma region /** Bossfight */
public:
	// 현재 게임모드가 속한 레벨이 보스전을 위한 레벨인지 여부
	// 판정은 던전 제너레이터의 클래스 타입을 통해 BeginPlay에 이루어진다
	// NOTE: 보스전은 일반 던전과 별개의 파이프라인을 거치며 게임 로직이 처리된다
	UPROPERTY(EditAnywhere)
	bool bIsLevelBossFight = false;

	// 보스가 존재하는 메인 룸
	// 보스 던전 생성자에 의해 설정된다
	// NOTE: 던전 생성자의 ExitingRoom에 해당된다; 즉 보스가 속한 방은 항상 리프 노드가 된다
	// NOTE: 방이 여러 개 존재하는 보스전의 경우에는 그 중 보스 몬스터가 생성되는 방을 의미한다
	// NOTE: 생성할 보스가 여러 개일 경우 모든 보스는 이 방 안의 스폰 포인트들에서만 생성될 수 있다
	class URoom* BossRoom = nullptr;

protected:
	// 보스 처치 여부
	// NOTE: 보스전 클리어 여부와 별개로, 전투가 없는 보스전을 만들 수 있다
	bool bBossSlain = false;

	// 보스전 클리어 여부; 일반적으로 직접 수정하는 대신 ClearBossfight()를 사용하는 게 권장된다
	// NOTE: 클리어란 다음 층으로 이동할 수 있음을 의미함
	bool bBossfightCleared = false;

	// 이 값이 true이면서 동시에 bBossSlain이 true라면 클리어 된 것으로 간주함
	bool bMustSlayBossToClear = true;

	// 쓰려뜨려야 하는 대상(들)이 존재할 경우 해당 액터를 참조
	// 만약 대상이 여러 개일 경우 모두 쓰러뜨려야 쓰러뜨린 것으로 간주
	TSet<class ABotCharacter*> SpawnedBosses;

	// 스폰 가능한 보스 몬스터들의 목록
	UPROPERTY(EditDefaultsOnly)
	TArray<FSpawnableMonsterData> SpawnableBossClasses;

private:
	// 만약 보스 생성이 필요 없는 보스전인 경우 아무것도 생성되지 않았더라도 true로 설정됨
	bool bBossGenerated = false;

protected:
	// 보스전에서 매 틱마다 처리해야 할 로직 수행
	void BossfightTick(float DeltaTime);

	// 클리어 상태 업데이트
	void UpdateBossfightState(float DeltaTime);

	// 보스 스폰
	void ManageBosses(float DeltaTime);

	// 현재 소환할 보스를 선택해 반환한다
	// 이때 반환 결과는 배열로, 해당 배열 내에 포함된 모든 대상이 소환 대상으로 취급된다
	TArray<TSubclassOf<class ABotCharacter>> GetRandomSpawnBossClasses(int32 SpawnDepth);

	// 보스 클리어 시 추가 로직
	void OnBossfightCleared();

public:
	FORCEINLINE bool HasClearedBossfight() const;

	// 보스몹이 사망하였을 경우 델리게이트로 호출된다; 
	// 래그돌이 남아있을 수 있으므로 파괴 대신 사망에 대한 이벤트 처리를 사용한다
	UFUNCTION()
	void OnSpawnedBossDeath(ACharacter* DeadCharacter);

	// 보스전을 클리어하기 위해 호출하는 유일한 진입점
	void ClearBossfight();
#pragma endregion

#pragma region /** UI */
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ADamageNumber> DefaultDamageNumberClass = nullptr;
#pragma endregion

#pragma region /** Debug */
#pragma endregion
};
