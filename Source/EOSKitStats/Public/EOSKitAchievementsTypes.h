// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitAchievementsTypes.generated.h"

/**
 * Achievement definition from EOS
 */
USTRUCT(BlueprintType)
struct FEOSAchievementDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString AchievementId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString LockedDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString LockedDescription;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString LockedIconUrl;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString UnlockedIconUrl;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	bool bIsHidden = false;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	int32 StatThresholdCount = 0;
};

/**
 * Player achievement progress and unlock status
 */
USTRUCT(BlueprintType)
struct FEOSPlayerAchievement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString AchievementId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	float Progress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FDateTime UnlockTime;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	bool bIsUnlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Achievements")
	FString IconURL;
};
