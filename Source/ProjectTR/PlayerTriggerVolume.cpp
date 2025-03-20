// Copyright (C) 2024 by Haguk Kim


#include "PlayerTriggerVolume.h"
#include "TRMacros.h"
#include "FPSCharacter.h"
#include "Components/BrushComponent.h"
#include "ProjectTRGameModeBase.h"

APlayerTriggerVolume::APlayerTriggerVolume()
{
    // 콜리전
    UBrushComponent* BrushComp = GetBrushComponent();
    if (BrushComp)
    {
        BrushComp->SetCollisionProfileName(TEXT("PlayerTriggerVolume"));
    }

    // 서버의 경우에만 델리게이트 등록
    if (HasAuthority())
    {
        OnActorBeginOverlap.AddDynamic(this, &APlayerTriggerVolume::Server_OnOverlapBegin);
        OnActorEndOverlap.AddDynamic(this, &APlayerTriggerVolume::Server_OnOverlapEnd);
    }
}

void APlayerTriggerVolume::Server_OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!HasAuthority()) return;
    if (OtherActor && (OtherActor != this))
    {
        TR_PRINT_FSTRING("Overlapped with %s", *OtherActor->GetName());
        AFPSCharacter* Player = Cast<AFPSCharacter>(OtherActor);
        if (IsValid(Player))
        {
            Server_OnPlayerOverlapBegin(Player);
        }
    }

    ///////////TESTING
    if (IsAllPlayerOverlapped())
    {
        TR_PRINT("All player overlapped!");
    }
}

void APlayerTriggerVolume::Server_OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!HasAuthority()) return;
    if (OtherActor && (OtherActor != this))
    {
        TR_PRINT_FSTRING("Overlap ended with %s", *OtherActor->GetName());
        AFPSCharacter* Player = Cast<AFPSCharacter>(OtherActor);
        if (IsValid(Player))
        {
            Server_OnPlayerOverlapEnd(Player);
        }
    }
}

bool APlayerTriggerVolume::IsAllPlayerOverlapped()
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("IsAllPlayerOverlapped - This is not intended to be called from non-authoritative host."));
        return false;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        AProjectTRGameModeBase* TRGameMode = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
        if (TRGameMode)
        {
            // 살아있는 플레이어 캐릭터들의 목록을 가져와 그 크기를 구한다
            int PlayerCount = TRGameMode->GetInGamePlayerCharacters(true).Num();
            if (PlayerCount <= OverlappedPlayerCount)
            {
                if (PlayerCount < OverlappedPlayerCount)
                {
                    UE_LOG(LogTemp, Error, TEXT("IsAllPlayerOverlapped - OverlappedPlayerCount is larger than the actual player count. This should not happen."));
                }
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    UE_LOG(LogTemp, Error, TEXT("IsAllPlayerOverlapped - Something unexpected happen. Please check."));
    return false;
}

void APlayerTriggerVolume::Server_OnPlayerOverlapBegin(AFPSCharacter* Player)
{
    if (!Player) return;
    ++OverlappedPlayerCount;

    if (bActivateOnceOnly && bHasActivatedOnceOrMore) return;

    bool bAllPlayerOverlapped = IsAllPlayerOverlapped();

    // 트리거 실행
    if (bTriggerPairWhenPlayerOverlap)
    {
        if (Pair.IsValid())
        {
            Pair->TriggerThis();
        }

        bHasActivatedOnceOrMore = true;
    }
    else if (bTriggerPairWhenAllPlayerOverlap && bAllPlayerOverlapped)
    {
        if (Pair.IsValid())
        {
            Pair->TriggerThis();
        }

        bHasActivatedOnceOrMore = true;
    }

    // 커스텀 로직 실행
    ProcessPlayerOverlap(Player, bAllPlayerOverlapped);
    if (bAllPlayerOverlapped)
    {
        ProcessAllPlayersOverlap();
    }
}

void APlayerTriggerVolume::Server_OnPlayerOverlapEnd(AFPSCharacter* Player)
{
    if (!Player) return;
    --OverlappedPlayerCount;
    if (OverlappedPlayerCount < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Server_OnPlayerOverlapEnd - OverlappedPlayerCount should never go under zero!"));
        OverlappedPlayerCount = 0;
    }
}

void APlayerTriggerVolume::ProcessPlayerOverlap(AFPSCharacter* Player, bool bAllPlayersOverlapped)
{
    // 필요 시 오버라이드
}

void APlayerTriggerVolume::ProcessAllPlayersOverlap()
{
    // 필요 시 오버라이드
}
