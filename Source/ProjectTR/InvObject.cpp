// Copyright (C) 2024 by Haguk Kim


#include "InvObject.h"
#include "ItemData.h"
#include "BaseItem.h"
#include "IconStageActor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Templates/Tuple.h"
#include "TRMacros.h"
#include "ProjectTRGameModeBase.h"

UInvObject::UInvObject()
{
    static ConstructorHelpers::FObjectFinder<UMaterial> MatAsset(TEXT(ASSET_DEFAULT_ICON_MATERIAL));
    if (MatAsset.Succeeded())
    {
        BaseIconMaterial = MatAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UInvObject - Failed to load base material asset!"));
    }
}

void UInvObject::SetItemData(UItemData* Data)
{
    ItemData = Data;
}

void UInvObject::SetBaseItemClass(TSubclassOf<ABaseItem> Class)
{
    BaseItemClass = Class;
}

ABaseItem* UInvObject::GenerateAndSpawnItem(UObject* Outer, FVector Location, FRotator Rotation, FActorSpawnParameters Params, bool bRestoreUsingItemData)
{
    UClass* ItemClass = BaseItemClass;
    ABaseItem* GeneratedItem = nullptr;
    UWorld* World = Outer->GetWorld();

    if (!IsValid(ItemClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("InvObject %s has no ItemDataClass set. Something went wrong during passing GetClass() from ABaseItem."), *GetName());
        return nullptr;
    }
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Warning, TEXT("InvObject %s has no world."), *GetName());
        return nullptr;
    }

    AProjectTRGameModeBase* GameMode = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("InvObject %s is unable to get appropriate GameMode."), *GetName());
        return nullptr;
    }

    if (bRestoreUsingItemData)
    {
        GeneratedItem = GameMode->RespawnItem(ItemClass, World, Location, Rotation, Params, this, ItemData);
    }
    else
    {
        GeneratedItem = GameMode->SpawnItem(ItemClass, World, Location, Rotation, Params);
    }
    GeneratedItem->SetInvObject(this);
    GeneratedItem->SetItemData(ItemData);
    return GeneratedItem;
}

void UInvObject::ChangeRootSetRecursive(bool bAddRoot, UObject* NewOuter)
{
    if (bAddRoot) this->AddToRoot();
    else this->RemoveFromRoot();

    if (NewOuter) Rename(nullptr, NewOuter);

    if (ItemData)
    {
        ItemData->ChangeRootSetRecursive(bAddRoot, NewOuter);
    }
}

FInvObjSize UInvObject::GetDimensions() const
{
    return { InvXSize, InvYSize };
}

UMaterialInstance* UInvObject::GetCurrIcon() const
{
    // TODO: Rotation
    return GetIcon();
}

UMaterialInstance* UInvObject::GetIcon() const
{
    if (IconMat) return IconMat.Get();
    return nullptr;
}

void UInvObject::Host_ProcessRefreshIcon(AIconStageActor* TargetActor)
{
    if (bUseStaticBoundIcon) return;

    // 렌더타깃 스냅샷 캡처
    UTextureRenderTarget2D* TextureTarget = TargetActor->CreateIconRenderTarget(GetDimensions().X * INV_GRID_PIXEL, GetDimensions().Y * INV_GRID_PIXEL);
    TargetActor->SetTextureTargetAs(TextureTarget);
    TargetActor->CaptureTarget();

    // 렌더타깃을 표기하기 위해서는 Dynamic material을 사용해야 한다
    // 이미 기존에 생성한 전력이 있는 경우 새로 만드는 대신 파라미터 값만 수정한다
    if (!bIsIconMatDynamic)
    {
        IconMat = UMaterialInstanceDynamic::Create(BaseIconMaterial, this);
        bIsIconMatDynamic = true;
    }
    UMaterialInstanceDynamic* DynamicMat = Cast<UMaterialInstanceDynamic>(IconMat);
    if (!DynamicMat)
    {
        UE_LOG(LogTemp, Error, TEXT("Host_ProcessRefreshIcon - Something went wrong!"));
        return;
    }
    DynamicMat->SetTextureParameterValue(FName("Texture"), TextureTarget);
}

void UInvObject::SetCurrIconStageActor(AIconStageActor* IconStageActor)
{
    CurrIconStageActor = IconStageActor;

    // Authority check
    if (GetWorld() && GetWorld()->GetAuthGameMode())
    {
        // 서버의 경우 직접 호출한다
        OnIconStageActorRepl();
    }
}

void UInvObject::OnIconStageActorRepl()
{
    if (CurrIconStageActor)
    {
        Host_ProcessRefreshIcon(CurrIconStageActor);
    }
    // NOTE: 액터가 파괴되도 아이콘은 그대로 유지한다
}

void UInvObject::Server_RequestUpdateIcon()
{
    UWorld* World = GetWorld();
    AProjectTRGameModeBase* TRGM = Cast<AProjectTRGameModeBase>(World->GetAuthGameMode());
    if (TRGM)
    {
        TRGM->UpdateIconOf(this);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Server_RequestUpdateIcon - Unable to get GameMode! Please check if the function is called on the server."));
    }
}
