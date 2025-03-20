// Copyright (C) 2024 by Haguk Kim

#pragma once

#include "CoreMinimal.h"

/*
기능에 따른 신체 부위를 나타내는 Enum
특정 기능에 대한 부위를 맵핑하기 위해 사용할 수 있음
e.g. 아이템 장착 소켓을 구할 때 아이템의 장착 위치가 Primary Wield라면, 인간형 캐릭터는 hand_r로 맵핑될 수 있음
*/
UENUM(BlueprintType)
enum class ECharacterParts : uint8 
{
	ECP_CentralNerves UMETA(DisplayName = "Central Nerve System"), // 주로 머리에 해당
	ECP_PrimaryWield UMETA(DisplayName = "Primary Wield"), // 주로 손에 해당
	ECP_SecondaryWield UMETA(DisplayName = "Secondary Wield"), 
};

/* 
총기 파츠의 메쉬 종류를 나타내는 Enum
*/
UENUM(BlueprintType)
enum class EGunPartMeshType : uint8
{
	EGM_Invalid UMETA(DisplayName = "Invalid mesh component type"),
	EGM_Static UMETA(DisplayName = "Static mesh component"),
	EGM_Skeletal UMETA(DisplayName = "Skeletal mesh component"),
};

/*
총기 파츠 종류를 나타내는 Enum
*/
UENUM(BlueprintType)
enum class EGunPartType : uint8
{
	EGT_Undefined UMETA(DisplayName = "Undefined"),
	EGT_Receiver UMETA(DisplayName = "Receiver"),
	EGT_Barrel UMETA(DisplayName = "Barrel"),
	EGT_Grip UMETA(DisplayName = "Grip"),
	EGT_Magazine UMETA(DisplayName = "Magazine"),
	EGT_Muzzle UMETA(DisplayName = "Muzzle"),
	EGT_Sight UMETA(DisplayName = "Sight"),
	EGT_Stock UMETA(DisplayName = "Stock"),
};

/*
애님클래스 종류를 나타내는 Enum
*/
UENUM(BlueprintType)
enum class EAnimClassType : uint8
{
	AC_UNARMED UMETA(DisplayName = "Unarmed anim class"),
	AC_LIGHTWEAPON UMETA(DisplayName = "Light weapon anim class"),
	AC_HEAVYWEAPON UMETA(DisplayName = "Heavy weapon anim class"),
};

/*
몬스터 스폰 주기 스테이트를 나타내는 Enum
*/
UENUM(BlueprintType)
enum class EEnemyGenerationState : uint8
{
	EGS_IDLE UMETA(DisplayName = "Maintain currently set spawn rate"),

	EGS_RISE UMETA(DisplayName = "Increase enemy spawn rate"),
	EGS_PEAK UMETA(DisplayName = "Maintain peak enemy spawn rate"),
	EGS_FALL UMETA(DisplayName = "Decrease enemy spawn rate"),
	EGS_RELAX UMETA(DisplayName = "Maintain minimum enemy spawn rate"),

	EGS_RED UMETA(DisplayName = "Dungeon exit time has exceeded; maintain max spawn rate"),
};

/*
Weapon fire 모드를 나타내는 Enum
*/
UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	WFM_SAFE UMETA(DisplayName = "Does not fire"),
	WFM_SINGLE UMETA(DisplayName = "Fire one time per shot"),
	WFM_AUTO UMETA(DisplayName = "Fire continuously"),
	WFM_BURST UMETA(DisplayName = "Fire X amount per shot"),
};

/*
Weapon fire 타입을 나타내는 Enum
*/
UENUM(BlueprintType)
enum class EWeaponFireType : uint8
{
	WFT_HITSCAN UMETA(DisplayName = "Weapon immediately hits the target"),
	WFT_PROJECTILE UMETA(DisplayName = "Weapon fires a physical object"),
};

/* 봇 애니메이션 타입을 나타내는 Enum */
UENUM(BlueprintType)
enum class EBotAnimType : uint8
{
	BAT_MELEE UMETA(DisplayName = "Melee attack animation"),
	BAT_RANGED UMETA(DiplayName = "Ranged attack animation"),
};

/* Item Attribute 타입을 나타내는 Enum */
UENUM(BlueprintType)
enum class EItemAttrType : uint8
{
	// UI 표시순서대로 나열
	// 강조된 부분을 먼저 표기
	IAT_NEUTRAL_MAJOR UMETA(DisplayName = "neutral major"),
	IAT_NEUTRAL_NORMAL UMETA(DisplayName = "neutral normal"),
	IAT_NEUTRAL_MINOR UMETA(DisplayName = "neutral minor"), // Major 혹은 Normal의 하위 어트리뷰트를 표현하기 위해 사용
	IAT_POSITIVE UMETA(DisplayName = "positive"),
	IAT_NEGATIVE UMETA(DisplayName = "negative"),
};
