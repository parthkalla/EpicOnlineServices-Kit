// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSanctionsTypes.generated.h"

/**
 * Sanction Appeal Reason enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitSanctionAppealReason : uint8
{
	Invalid = 0					UMETA(DisplayName = "Invalid"),
	IncorrectSanction = 1		UMETA(DisplayName = "Incorrect Sanction"),
	CompromisedAccount = 2		UMETA(DisplayName = "Compromised Account"),
	UnfairPunishment = 3		UMETA(DisplayName = "Unfair Punishment"),
	AppealForForgiveness = 4	UMETA(DisplayName = "Appeal For Forgiveness")
};

/**
 * Player Sanction information
 */
USTRUCT(BlueprintType, Category = "EOSKit|Sanctions")
struct EOSKITECOM_API FEOSKitPlayerSanction
{
	GENERATED_BODY()

	/** The POSIX timestamp when the sanction was placed */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Sanctions")
	int64 TimePlaced = 0;

	/** The action associated with this sanction */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Sanctions")
	FString Action = TEXT("");

	/** The POSIX timestamp when the sanction will expire. If the sanction is permanent, this will be 0. */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Sanctions")
	int64 TimeExpires = 0;

	/** A unique identifier for this specific sanction */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Sanctions")
	FString ReferenceId = TEXT("");

	/** True if the sanction is permanent (TimeExpires == 0) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Sanctions")
	bool bIsPermanent = false;
};

