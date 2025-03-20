// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"

/* Utility */
#define TR_PRINT(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)
#define TR_PRINT_FSTRING(text, fstring) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT(text), fstring))

/* System */
#define TR_MAX_PLAYER_COUNT 5
#define TR_PROJ_MOVEMENT_TICKRATE 0.033f
#define TR_PROJ_MOVEREPL_RATE 20
#define TR_BOT_MOVECOMP_TICKRATE 0.033f

/* Niagara */
#define TR_NIAGARA_MAX_HIT_VFX_CNT 32
#define TR_NIAGARA_MAX_TRACER_VFX_CNT 32
#define TR_NIAGARA_MAX_EXPLOSION_VFX_CNT 32

/* General */
#define TR_LONG_LINETRACE_DIST 100000.f
#define TR_DIST_CLOSE_TO_PLAYER_HEIGHT 200.f
#define TR_DEFAULT_HITSCAN_RECURSION 32
#define TR_MAX_HITSCAN_RECURSION 64

/* Gameplay */
#define TR_GUN_LONG_DIST 2000.0f
#define TR_GUN_CLOSE_DIST 250.0f
#define TR_DUNGEON_MIN_LOCKED_DOORS_PER_LEVEL 1
#define TR_DUNGEON_MAX_LOCKED_DOORS_PER_LEVEL 3

/* Collision Channels */
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Item ECollisionChannel::ECC_GameTraceChannel2
#define ECC_Interactive ECollisionChannel::ECC_GameTraceChannel3
#define ECC_Hitbox ECollisionChannel::ECC_GameTraceChannel4
#define ECC_Altar ECollisionChannel::ECC_GameTraceChannel5
#define ECC_GunPart ECollisionChannel::ECC_GameTraceChannel6
#define ECC_Decoration ECollisionChannel::ECC_GameTraceChannel7
#define ECC_Explosion ECollisionChannel::ECC_GameTraceChannel8
#define ECC_Environmental ECollisionChannel::ECC_GameTraceChannel9
#define ECC_OuterHitbox ECollisionChannel::ECC_GameTraceChannel10
#define ECC_HitscanLinetrace ECollisionChannel::ECC_GameTraceChannel11
#define ECC_VirtualTarget ECollisionChannel::ECC_GameTraceChannel12
#define ECC_PlayerPawn ECollisionChannel::ECC_GameTraceChannel13
#define ECC_BotPawn ECollisionChannel::ECC_GameTraceChannel14

// TODO: 프로젝트 세팅 ECC와 대조 확인

/******************** Level ********************/
#define LVL_DUNGEON_ASSET "/Game/Game/Maps/LVL_Dungeon"

/********************* FX **********************/
#define ASSET_DEFAULT_FX "/Game/Assets/FxConfig.FxConfig"
#define ASSET_DEFAULT_CAMSHAKE "/Game/Assets/CamShakeConfig.CamShakeConfig"
#define ASSET_DEFAULT_PROJECTILE "/Game/Assets/ProjectileConfig.ProjectileConfig"
#define ASSET_DEFAULT_BULLET "/Game/Assets/BulletConfig.BulletConfig"
#define ASSET_DEFAULT_AUDIO "/Game/Assets/AudioConfig.AudioConfig"
#define ASSET_DEFAULT_GUN_SOUND_ATTENUATION "/Game/Audio/SA_GunfireDefault.SA_GunfireDefault"
#define ASSET_DEFAULT_GUN_SOUND_CONCURRENCY "/Game/Audio/SC_GunfireDefault.SC_GunfireDefault"

/********************* UI *********************/
/* Widget Z-order */
#define WZO_DEFAULT 0
#define WZO_HUD 1
#define WZO_INV 2
#define WZO_SHOP 4
#define WZO_ALERT 1024

/* 인벤토리 빈칸 식별 id */
#define INV_EMPTY 0

/* UI RefreshCount 최대 허용 오차 */
#define MAX_REFR_DIFF 100

/* 인벤토리 격자 당 픽셀 크기 */
#define INV_GRID_PIXEL 72.0f

/* 인벤토리 아이템 아이콘 생성 시 사용할 베이스 매터리얼 */
#define ASSET_DEFAULT_ICON_MATERIAL "/Game/VFX/M_IconObject.M_IconObject"

/********************* Actors *******************/
#define DEFAULT_TOKEN_ITEM "/Game/Game/Blueprint/Items/BP_Token.BP_Token_C"
#define DEFAULT_BULLET_TRACER_PROJECTILE "/Game/Game/Blueprint/BP_BulletTracer.BP_BulletTracer_C"
#define DEFAULT_WEAPON_PROJECTILE "/Game/Game/Blueprint/BP_DefaultProjectile.BP_DefaultProjectile_C"

/******************** Recoil ******************/
#define DEFAULT_RECOIL_LIGHT_ASSET "/Game/PRAS/DefaultLightRecoilAsset.DefaultLightRecoilAsset"
#define DEFAULT_RECOIL_LIGHT_LOC "/Game/PRAS/DefaultLightLoc.DefaultLightLoc"
#define DEFAULT_RECOIL_LIGHT_ROT "/Game/PRAS/DefaultLightRot.DefaultLightRot"
#define DEFAULT_RECOIL_HEAVY_ASSET "/Game/PRAS/DefaultHeavyRecoilAsset.DefaultHeavyRecoilAsset"
#define DEFAULT_RECOIL_HEAVY_LOC "/Game/PRAS/DefaultHeavyLoc.DefaultHeavyLoc"
#define DEFAULT_RECOIL_HEAVY_ROT "/Game/PRAS/DefaultHeavyRot.DefaultHeavyRot"

/*************** Gun Generation **************/
/* Gun generation socket names */
#define BARREL_SOCKET "BarrelSocket" // 리시버에 배럴을 붙이는 소켓
#define GRIP_SOCKET "GripSocket" // 리시버에 그립을 붙이는 소켓
#define MAG_SOCKET "MagSocket" // 리시버에 탄창을 붙이는 소켓
#define STOCK_SOCKET "StockSocket" // 리시버에 스톡을 붙이는 소켓
#define MUZZLE_SOCKET "BarrelEndSocket" // 배럴에 머즐을 붙이는 소켓
#define SIGHT_SOCKET "PicatinySightSocket" // 배럴에 사이트를 붙이는 소켓

/* General socket names */
#define MELEE_ATK_SOCKET_R "MeleeAtkSocket_R" // 밀리 공격 히트 판정을 위해 사용하는 소켓
#define MELEE_ATK_SOCKET_L "MeleeAtkSocket_L"

/* Gun generation asset references */
// Receiver
#define SK_RECEIVER_DOUBLEBARREL_1 "/Game/Items/Guns/Receiver/SK/GG_Receiver_DoubleBarrel1.GG_Receiver_DoubleBarrel1"
#define SK_RECEIVER_HANDGUN_1 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Handgun1.GG_Receiver_Handgun1"
#define SK_RECEIVER_PISTOL_1 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Pistol1.GG_Receiver_Pistol1"
#define SK_RECEIVER_PISTOL_2 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Pistol2.GG_Receiver_Pistol2"
#define SK_RECEIVER_REVOLVER_1 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Revolver1.GG_Receiver_Revolver1"
#define SK_RECEIVER_RIFLE_1 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Rifle1.GG_Receiver_Rifle1"
#define SK_RECEIVER_RIFLE_2 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Rifle2.GG_Receiver_Rifle2"
#define SK_RECEIVER_RIFLE_3 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Rifle3.GG_Receiver_Rifle3"
#define SK_RECEIVER_RIFLE_4 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Rifle4.GG_Receiver_Rifle4"
#define SK_RECEIVER_SHOTGUN_1 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Shotgun1.GG_Receiver_Shotgun1"
#define SK_RECEIVER_SHOTGUN_2 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Shotgun2.GG_Receiver_Shotgun2"
#define SK_RECEIVER_SHOTGUN_3 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Shotgun3.GG_Receiver_Shotgun3"
#define SK_RECEIVER_SMG_1 "/Game/Items/Guns/Receiver/SK/GG_Receiver_SMG1.GG_Receiver_SMG1"
#define SK_RECEIVER_SMG_2 "/Game/Items/Guns/Receiver/SK/GG_Receiver_SMG2.GG_Receiver_SMG2"
#define SK_RECEIVER_SMG_3 "/Game/Items/Guns/Receiver/SK/GG_Receiver_SMG3.GG_Receiver_SMG3"
#define SK_RECEIVER_SMG_4 "/Game/Items/Guns/Receiver/SK/GG_Receiver_SMG4.GG_Receiver_SMG4"
#define SK_RECEIVER_SNIPER_1 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Sniper1.GG_Receiver_Sniper1"
#define SK_RECEIVER_SNIPER_2 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Sniper2.GG_Receiver_Sniper2"
#define SK_RECEIVER_SNIPER_3 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Sniper3.GG_Receiver_Sniper3"
#define SK_RECEIVER_SNIPER_4 "/Game/Items/Guns/Receiver/SK/GG_Receiver_Sniper4.GG_Receiver_Sniper4"

// Barrel
#define SK_BARREL_DOUBLEBARREL_1 "/Game/Items/Guns/Barrel/SK/GG_Barrel_DoubleBarrel1.GG_Barrel_DoubleBarrel1"
#define SK_BARREL_DOUBLEBARREL_2 "/Game/Items/Guns/Barrel/SK/GG_Barrel_DoubleBarrel2.GG_Barrel_DoubleBarrel2"
#define SK_BARREL_DOUBLEBARREL_3 "/Game/Items/Guns/Barrel/SK/GG_Barrel_DoubleBarrel3.GG_Barrel_DoubleBarrel3"
#define SK_BARREL_PISTOL_1 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Pistol1.GG_Barrel_Pistol1"
#define SK_BARREL_PISTOL_2 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Pistol2.GG_Barrel_Pistol2"
#define SK_BARREL_PISTOL_3 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Pistol3.GG_Barrel_Pistol3"
#define SK_BARREL_REVOLVER_1 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Revolver1.GG_Barrel_Revolver1"
#define SK_BARREL_REVOLVER_2 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Revolver2.GG_Barrel_Revolver2"
#define SK_BARREL_REVOLVER_3 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Revolver3.GG_Barrel_Revolver3"
#define SK_BARREL_RIFLE_1 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle1.GG_Barrel_Rifle1"
#define SK_BARREL_RIFLE_2 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle2.GG_Barrel_Rifle2"
#define SK_BARREL_RIFLE_3 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle3.GG_Barrel_Rifle3"
#define SK_BARREL_RIFLE_4 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle4.GG_Barrel_Rifle4"
#define SK_BARREL_RIFLE_5 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle5.GG_Barrel_Rifle5"
#define SK_BARREL_RIFLE_6 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle6.GG_Barrel_Rifle6"
#define SK_BARREL_RIFLE_7 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle7.GG_Barrel_Rifle7"
#define SK_BARREL_RIFLE_8 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle8.GG_Barrel_Rifle8"
#define SK_BARREL_RIFLE_9 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle9.GG_Barrel_Rifle9"
#define SK_BARREL_RIFLE_10 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle10.GG_Barrel_Rifle10"
#define SK_BARREL_RIFLE_11 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle11.GG_Barrel_Rifle11"
#define SK_BARREL_RIFLE_12 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Rifle12.GG_Barrel_Rifle12"
#define SK_BARREL_SHOTGUN_1 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Shotgun1.GG_Barrel_Shotgun1"
#define SK_BARREL_SHOTGUN_2 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Shotgun2.GG_Barrel_Shotgun2"
#define SK_BARREL_SHOTGUN_3 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Shotgun3.GG_Barrel_Shotgun3"
#define SK_BARREL_SHOTGUN_4 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Shotgun4.GG_Barrel_Shotgun4"
#define SK_BARREL_SHOTGUN_5 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Shotgun5.GG_Barrel_Shotgun5"
#define SK_BARREL_SHOTGUN_6 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Shotgun6.GG_Barrel_Shotgun6"
#define SK_BARREL_SHOTGUN_7 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Shotgun7.GG_Barrel_Shotgun7"
#define SK_BARREL_SHOTGUN_8 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Shotgun8.GG_Barrel_Shotgun8"
#define SK_BARREL_SMG_1 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG1.GG_Barrel_SMG1"
#define SK_BARREL_SMG_2 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG2.GG_Barrel_SMG2"
#define SK_BARREL_SMG_3 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG3.GG_Barrel_SMG3"
#define SK_BARREL_SMG_4 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG4.GG_Barrel_SMG4"
#define SK_BARREL_SMG_5 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG5.GG_Barrel_SMG5"
#define SK_BARREL_SMG_6 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG6.GG_Barrel_SMG6"
#define SK_BARREL_SMG_7 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG7.GG_Barrel_SMG7"
#define SK_BARREL_SMG_8 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG8.GG_Barrel_SMG8"
#define SK_BARREL_SMG_9 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG9.GG_Barrel_SMG9"
#define SK_BARREL_SMG_10 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG10.GG_Barrel_SMG10"
#define SK_BARREL_SMG_11 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG11.GG_Barrel_SMG11"
#define SK_BARREL_SMG_12 "/Game/Items/Guns/Barrel/SK/GG_Barrel_SMG12.GG_Barrel_SMG12"
#define SK_BARREL_SNIPER_1 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper1.GG_Barrel_Sniper1"
#define SK_BARREL_SNIPER_2 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper2.GG_Barrel_Sniper2"
#define SK_BARREL_SNIPER_3 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper3.GG_Barrel_Sniper3"
#define SK_BARREL_SNIPER_4 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper4.GG_Barrel_Sniper4"
#define SK_BARREL_SNIPER_5 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper5.GG_Barrel_Sniper5"
#define SK_BARREL_SNIPER_6 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper6.GG_Barrel_Sniper6"
#define SK_BARREL_SNIPER_7 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper7.GG_Barrel_Sniper7"
#define SK_BARREL_SNIPER_8 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper8.GG_Barrel_Sniper8"
#define SK_BARREL_SNIPER_9 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper9.GG_Barrel_Sniper9"
#define SK_BARREL_SNIPER_10 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper10.GG_Barrel_Sniper10"
#define SK_BARREL_SNIPER_11 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper11.GG_Barrel_Sniper11"
#define SK_BARREL_SNIPER_12 "/Game/Items/Guns/Barrel/SK/GG_Barrel_Sniper12.GG_Barrel_Sniper12"

// Grip
#define SM_GRIP_PISTOL_1 "/Game/Items/Guns/Grip/SM/GG_Grip_Pistol1.GG_Grip_Pistol1"
#define SM_GRIP_PISTOL_2 "/Game/Items/Guns/Grip/SM/GG_Grip_Pistol2.GG_Grip_Pistol2"
#define SM_GRIP_REVOLVER_1 "/Game/Items/Guns/Grip/SM/GG_Grip_Revolver1.GG_Grip_Revolver1"
#define SM_GRIP_REVOLVER_2 "/Game/Items/Guns/Grip/SM/GG_Grip_Revolver2.GG_Grip_Revolver2"
#define SM_GRIP_REVOLVER_3 "/Game/Items/Guns/Grip/SM/GG_Grip_Revolver3.GG_Grip_Revolver3"
#define SM_GRIP_RIFLE_1 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle1.GG_Grip_Rifle1"
#define SM_GRIP_RIFLE_2 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle2.GG_Grip_Rifle2"
#define SM_GRIP_RIFLE_3 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle3.GG_Grip_Rifle3"
#define SM_GRIP_RIFLE_4 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle4.GG_Grip_Rifle4"
#define SM_GRIP_RIFLE_5 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle5.GG_Grip_Rifle5"
#define SM_GRIP_RIFLE_6 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle6.GG_Grip_Rifle6"
#define SM_GRIP_RIFLE_7 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle7.GG_Grip_Rifle7"
#define SM_GRIP_RIFLE_8 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle8.GG_Grip_Rifle8"
#define SM_GRIP_RIFLE_9 "/Game/Items/Guns/Grip/SM/GG_Grip_Rifle9.GG_Grip_Rifle9"
#define SM_GRIP_SHOTGUN_1 "/Game/Items/Guns/Grip/SM/GG_Grip_Shotgun1.GG_Grip_Shotgun1"
#define SM_GRIP_SHOTGUN_2 "/Game/Items/Guns/Grip/SM/GG_Grip_Shotgun2.GG_Grip_Shotgun2"
#define SM_GRIP_SHOTGUN_3 "/Game/Items/Guns/Grip/SM/GG_Grip_Shotgun3.GG_Grip_Shotgun3"
#define SM_GRIP_SHOTGUN_4 "/Game/Items/Guns/Grip/SM/GG_Grip_Shotgun4.GG_Grip_Shotgun4"
#define SM_GRIP_SHOTGUN_5 "/Game/Items/Guns/Grip/SM/GG_Grip_Shotgun5.GG_Grip_Shotgun5"
#define SM_GRIP_SMG_1 "/Game/Items/Guns/Grip/SM/GG_Grip_SMG1.GG_Grip_SMG1"
#define SM_GRIP_SMG_2 "/Game/Items/Guns/Grip/SM/GG_Grip_SMG2.GG_Grip_SMG2"
#define SM_GRIP_SMG_3 "/Game/Items/Guns/Grip/SM/GG_Grip_SMG3.GG_Grip_SMG3"
#define SM_GRIP_SMG_4 "/Game/Items/Guns/Grip/SM/GG_Grip_SMG4.GG_Grip_SMG4"
#define SM_GRIP_SMG_5 "/Game/Items/Guns/Grip/SM/GG_Grip_SMG5.GG_Grip_SMG5"
#define SM_GRIP_SMG_6 "/Game/Items/Guns/Grip/SM/GG_Grip_SMG6.GG_Grip_SMG6"
#define SM_GRIP_SMG_7 "/Game/Items/Guns/Grip/SM/GG_Grip_SMG7.GG_Grip_SMG7"
#define SM_GRIP_SMG_8 "/Game/Items/Guns/Grip/SM/GG_Grip_SMG8.GG_Grip_SMG8"
#define SM_GRIP_SNIPER_1 "/Game/Items/Guns/Grip/SM/GG_Grip_Sniper1.GG_Grip_Sniper1"
#define SM_GRIP_SNIPER_2 "/Game/Items/Guns/Grip/SM/GG_Grip_Sniper2.GG_Grip_Sniper2"
#define SM_GRIP_SNIPER_3 "/Game/Items/Guns/Grip/SM/GG_Grip_Sniper3.GG_Grip_Sniper3"
#define SM_GRIP_SNIPER_4 "/Game/Items/Guns/Grip/SM/GG_Grip_Sniper4.GG_Grip_Sniper4"

// Stock
#define SM_STOCK_DOUBLEBARREL_1 "/Game/Items/Guns/Stock/SM/GG_Stock_DoubleBarrel1.GG_Stock_DoubleBarrel1"
#define SM_STOCK_DOUBLEBARREL_2 "/Game/Items/Guns/Stock/SM/GG_Stock_DoubleBarrel2.GG_Stock_DoubleBarrel2"
#define SM_STOCK_PISTOL_1 "/Game/Items/Guns/Stock/SM/GG_Stock_Pistol1.GG_Stock_Pistol1"
#define SM_STOCK_RIFLE_1 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle1.GG_Stock_Rifle1"
#define SM_STOCK_RIFLE_2 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle2.GG_Stock_Rifle2"
#define SM_STOCK_RIFLE_3 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle3.GG_Stock_Rifle3"
#define SM_STOCK_RIFLE_4 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle4.GG_Stock_Rifle4"
#define SM_STOCK_RIFLE_5 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle5.GG_Stock_Rifle5"
#define SM_STOCK_RIFLE_6 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle6.GG_Stock_Rifle6"
#define SM_STOCK_RIFLE_7 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle7.GG_Stock_Rifle7"
#define SM_STOCK_RIFLE_8 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle8.GG_Stock_Rifle8"
#define SM_STOCK_RIFLE_9 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle9.GG_Stock_Rifle9"
#define SM_STOCK_RIFLE_10 "/Game/Items/Guns/Stock/SM/GG_Stock_Rifle10.GG_Stock_Rifle10"
#define SM_STOCK_SHOTGUN_1 "/Game/Items/Guns/Stock/SM/GG_Stock_Shotgun1.GG_Stock_Shotgun1"
#define SM_STOCK_SHOTGUN_2 "/Game/Items/Guns/Stock/SM/GG_Stock_Shotgun2.GG_Stock_Shotgun2"
#define SM_STOCK_SHOTGUN_3 "/Game/Items/Guns/Stock/SM/GG_Stock_Shotgun3.GG_Stock_Shotgun3"
#define SM_STOCK_SHOTGUN_4 "/Game/Items/Guns/Stock/SM/GG_Stock_Shotgun4.GG_Stock_Shotgun4"
#define SM_STOCK_SMG_1 "/Game/Items/Guns/Stock/SM/GG_Stock_SMG1.GG_Stock_SMG1"
#define SM_STOCK_SMG_2 "/Game/Items/Guns/Stock/SM/GG_Stock_SMG2.GG_Stock_SMG2"
#define SM_STOCK_SMG_3 "/Game/Items/Guns/Stock/SM/GG_Stock_SMG3.GG_Stock_SMG3"
#define SM_STOCK_SMG_4 "/Game/Items/Guns/Stock/SM/GG_Stock_SMG4.GG_Stock_SMG4"
#define SM_STOCK_SNIPER_1 "/Game/Items/Guns/Stock/SM/GG_Stock_Sniper1.GG_Stock_Sniper1"
#define SM_STOCK_SNIPER_2 "/Game/Items/Guns/Stock/SM/GG_Stock_Sniper2.GG_Stock_Sniper2"
#define SM_STOCK_SNIPER_3 "/Game/Items/Guns/Stock/SM/GG_Stock_Sniper3.GG_Stock_Sniper3"
#define SM_STOCK_SNIPER_4 "/Game/Items/Guns/Stock/SM/GG_Stock_Sniper4.GG_Stock_Sniper4"
#define SM_STOCK_SNIPER_5 "/Game/Items/Guns/Stock/SM/GG_Stock_Sniper5.GG_Stock_Sniper5"
#define SM_STOCK_SNIPER_6 "/Game/Items/Guns/Stock/SM/GG_Stock_Sniper6.GG_Stock_Sniper6"

// Magazine
#define SM_MAGAZINE_CURVED_1 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Curved1.GG_MagE_Curved1"
#define SM_MAGAZINE_CURVED_2 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Curved2.GG_MagE_Curved2"
#define SM_MAGAZINE_CURVED_STACK_1 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_CurvedStack1.GG_MagE_CurvedStack1"
#define SM_MAGAZINE_CURVED_STACK_2 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_CurvedStack2.GG_MagE_CurvedStack2"
#define SM_MAGAZINE_PISTOL_1 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Pistol1.GG_MagE_Pistol1"
#define SM_MAGAZINE_PISTOL_2 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Pistol2.GG_MagE_Pistol2"
#define SM_MAGAZINE_PISTOL_3 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Pistol3.GG_MagE_Pistol3"
#define SM_MAGAZINE_PISTOL_4 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Pistol4.GG_MagE_Pistol4"
#define SM_MAGAZINE_PISTOL_5 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Pistol5.GG_MagE_Pistol5"
#define SM_MAGAZINE_PISTOL_6 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Pistol6.GG_MagE_Pistol6"
#define SM_MAGAZINE_RIFLE_1 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Rifle1.GG_MagE_Rifle1"
#define SM_MAGAZINE_RIFLE_2 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Rifle2.GG_MagE_Rifle2"
#define SM_MAGAZINE_RIFLE_3 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Rifle3.GG_MagE_Rifle3"
#define SM_MAGAZINE_RIFLE_4 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Rifle4.GG_MagE_Rifle4"
#define SM_MAGAZINE_RIFLE_5 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Rifle5.GG_MagE_Rifle5"
#define SM_MAGAZINE_RIFLE_6 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Rifle6.GG_MagE_Rifle6"
#define SM_MAGAZINE_RIFLE_STACK_1 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_RifleStack1.GG_MagE_RifleStack1"
#define SM_MAGAZINE_RIFLE_STACK_2 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_RifleStack2.GG_MagE_RifleStack2"
#define SM_MAGAZINE_SHOTGUN_1 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Shotgun1.GG_MagE_Shotgun1"
#define SM_MAGAZINE_SHOTGUN_2 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Shotgun2.GG_MagE_Shotgun2"
#define SM_MAGAZINE_SHOTGUN_3 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Shotgun3.GG_MagE_Shotgun3"
#define SM_MAGAZINE_SMG_1 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_SMG1.GG_MagE_SMG1"
#define SM_MAGAZINE_SMG_2 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_SMG2.GG_MagE_SMG2"
#define SM_MAGAZINE_SMG_3 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_SMG3.GG_MagE_SMG3"
#define SM_MAGAZINE_SMG_4 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_SMG4.GG_MagE_SMG4"
#define SM_MAGAZINE_SMG_5 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_SMG5.GG_MagE_SMG5"
#define SM_MAGAZINE_SMG_6 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_SMG6.GG_MagE_SMG6"
#define SM_MAGAZINE_SNIPER_1 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Sniper1.GG_MagE_Sniper1"
#define SM_MAGAZINE_SNIPER_2 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Sniper2.GG_MagE_Sniper2"
#define SM_MAGAZINE_SNIPER_3 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Sniper3.GG_MagE_Sniper3"
#define SM_MAGAZINE_SNIPER_4 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Sniper4.GG_MagE_Sniper4"
#define SM_MAGAZINE_SNIPER_5 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Sniper5.GG_MagE_Sniper5"
#define SM_MAGAZINE_SNIPER_6 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Sniper6.GG_MagE_Sniper6"
#define SM_MAGAZINE_SNIPER_7 "/Game/Items/Guns/Magazine/Empty/SM/GG_MagE_Sniper7.GG_MagE_Sniper7"

// Muzzle
#define SM_MUZZLE_ANY_1 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any1.GG_Muzzle_Any1"
#define SM_MUZZLE_ANY_2 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any2.GG_Muzzle_Any2"
#define SM_MUZZLE_ANY_3 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any3.GG_Muzzle_Any3"
#define SM_MUZZLE_ANY_4 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any4.GG_Muzzle_Any4"
#define SM_MUZZLE_ANY_5 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any5.GG_Muzzle_Any5"
#define SM_MUZZLE_ANY_6 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any6.GG_Muzzle_Any6"
#define SM_MUZZLE_ANY_7 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any7.GG_Muzzle_Any7"
#define SM_MUZZLE_ANY_8 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any8.GG_Muzzle_Any8"
#define SM_MUZZLE_ANY_9 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any9.GG_Muzzle_Any9"
#define SM_MUZZLE_ANY_10 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any10.GG_Muzzle_Any10"
#define SM_MUZZLE_ANY_11 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Any11.GG_Muzzle_Any11"
#define SM_MUZZLE_PISTOL_1 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Pistol1.GG_Muzzle_Pistol1"
#define SM_MUZZLE_PISTOL_2 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Pistol2.GG_Muzzle_Pistol2"
#define SM_MUZZLE_RIFLE_1 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Rifle1.GG_Muzzle_Rifle1"
#define SM_MUZZLE_RIFLE_2 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Rifle2.GG_Muzzle_Rifle2"
#define SM_MUZZLE_RIFLE_3 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Rifle3.GG_Muzzle_Rifle3"
#define SM_MUZZLE_SHOTGUN_1 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Shotgun1.GG_Muzzle_Shotgun1"
#define SM_MUZZLE_SMG_1 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_SMG1.GG_Muzzle_SMG1"
#define SM_MUZZLE_SMG_2 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_SMG2.GG_Muzzle_SMG2"
#define SM_MUZZLE_SNIPER_1 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Sniper1.GG_Muzzle_Sniper1"
#define SM_MUZZLE_SNIPER_2 "/Game/Items/Guns/Muzzle/SM/GG_Muzzle_Sniper2.GG_Muzzle_Sniper2"

// Sight
#define SM_SIGHT_ACOG_1 "/Game/Items/Guns/Sight/SM/GG_Sight_ACOGSight1.GG_Sight_ACOGSight1"
#define SM_SIGHT_DOUBLEBARREL_1 "/Game/Items/Guns/Sight/SM/GG_Sight_DoubleBarrel1.GG_Sight_DoubleBarrel1"
#define SM_SIGHT_RDS_1 "/Game/Items/Guns/Sight/SM/GG_Sight_RDS1.GG_Sight_RDS1"
#define SM_SIGHT_RDS_2 "/Game/Items/Guns/Sight/SM/GG_Sight_RDS2.GG_Sight_RDS2"
#define SM_SIGHT_RDS_3 "/Game/Items/Guns/Sight/SM/GG_Sight_RDS3.GG_Sight_RDS3"
#define SM_SIGHT_RIFLE_1 "/Game/Items/Guns/Sight/SM/GG_Sight_Rifle1.GG_Sight_Rifle1"
#define SM_SIGHT_RIFLE_2 "/Game/Items/Guns/Sight/SM/GG_Sight_Rifle2.GG_Sight_Rifle2"
#define SM_SIGHT_RIFLE_3 "/Game/Items/Guns/Sight/SM/GG_Sight_Rifle3.GG_Sight_Rifle3"
#define SM_SIGHT_RIFLE_4 "/Game/Items/Guns/Sight/SM/GG_Sight_Rifle4.GG_Sight_Rifle4"
#define SM_SIGHT_RIFLE_5 "/Game/Items/Guns/Sight/SM/GG_Sight_Rifle5.GG_Sight_Rifle5"
#define SM_SIGHT_RIFLE_6 "/Game/Items/Guns/Sight/SM/GG_Sight_Rifle6.GG_Sight_Rifle6"
#define SM_SIGHT_RIFLE_7 "/Game/Items/Guns/Sight/SM/GG_Sight_Rifle7.GG_Sight_Rifle7"
#define SM_SIGHT_SCOPE_1 "/Game/Items/Guns/Sight/SM/GG_Sight_Scope1.GG_Sight_Scope1"
#define SM_SIGHT_SCOPE_2 "/Game/Items/Guns/Sight/SM/GG_Sight_Scope2.GG_Sight_Scope2"
#define SM_SIGHT_SHOTGUN_1 "/Game/Items/Guns/Sight/SM/GG_Sight_Shotgun1.GG_Sight_Shotgun1"
#define SM_SIGHT_SHOTGUN_2 "/Game/Items/Guns/Sight/SM/GG_Sight_Shotgun2.GG_Sight_Shotgun2"
#define SM_SIGHT_SHOTGUN_3 "/Game/Items/Guns/Sight/SM/GG_Sight_Shotgun3.GG_Sight_Shotgun3"
#define SM_SIGHT_SHOTGUN_4 "/Game/Items/Guns/Sight/SM/GG_Sight_Shotgun4.GG_Sight_Shotgun4"
#define SM_SIGHT_SHOTGUN_5 "/Game/Items/Guns/Sight/SM/GG_Sight_Shotgun5.GG_Sight_Shotgun5"
#define SM_SIGHT_SNIPER_1 "/Game/Items/Guns/Sight/SM/GG_Sight_Sniper1.GG_Sight_Sniper1"
#define SM_SIGHT_SNIPER_2 "/Game/Items/Guns/Sight/SM/GG_Sight_Sniper2.GG_Sight_Sniper2"
#define SM_SIGHT_SNIPER_3 "/Game/Items/Guns/Sight/SM/GG_Sight_Sniper3.GG_Sight_Sniper3"
#define SM_SIGHT_SNIPER_4 "/Game/Items/Guns/Sight/SM/GG_Sight_Sniper4.GG_Sight_Sniper4"
#define SM_SIGHT_SNIPER_5 "/Game/Items/Guns/Sight/SM/GG_Sight_Sniper5.GG_Sight_Sniper5"
#define SM_SIGHT_SNIPER_6 "/Game/Items/Guns/Sight/SM/GG_Sight_Sniper6.GG_Sight_Sniper6"
#define SM_SIGHT_SNIPER_7 "/Game/Items/Guns/Sight/SM/GG_Sight_Sniper7.GG_Sight_Sniper7"
#define SM_SIGHT_SNIPER_8 "/Game/Items/Guns/Sight/SM/GG_Sight_Sniper8.GG_Sight_Sniper8"

/*********************** AI ***********************/
/* Blackboard key names */
#define AI_TARGET "TargetActor" // 시각을 통해 선택한 타깃 액터
#define AI_PATROL_LOCATION "PatrolLocation" // 배회 목표 지점
#define AI_TARGET_LAST_KNOWN_LOCATION "TargetActor_LastKnownLocation" // 마지막 관측 지점
#define AI_IS_TARGET_LAST_KNOWN_LOC_VALID "IsTargetActorLastKnownLocationValid" // 현재 저장된 마지막 관측 지점이 유효한지 여부
#define AI_ATTACKER_LAST_KNOWN_LOCATION "Attacker_LastKnownLocation" // 공격자의 공격 당시 위치
#define AI_IS_ATTACKER_LAST_KNOWN_LOC_VALID "IsAttackerLastKnownLocationValid" // 현재 저장된 공격자 공격 위치가 유효한지 여부
#define AI_IS_MELEE_ANIM_PLAYING "IsMeleeAnimPlaying" // 현재 근접공격 애니메이션이 재생중인지 여부
#define AI_IS_RANGED_ANIM_PLAYING "IsRangedAnimPlaying" // 현재 원거리공격 애니메이션이 재생중인지 여부