// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitPresenceTypes.generated.h"

/**
 * Presence status enumeration matching EOS_Presence_EStatus
 */
UENUM(BlueprintType)
enum class EEOSKitPresenceStatus : uint8
{
	/** User is online */
	Online = 0 UMETA(DisplayName = "Online"),
	
	/** User is away */
	Away = 1 UMETA(DisplayName = "Away"),
	
	/** User is extended away */
	ExtendedAway = 2 UMETA(DisplayName = "Extended Away"),
	
	/** User is offline */
	Offline = 3 UMETA(DisplayName = "Offline"),
	
	/** User does not want to be disturbed */
	DoNotDisturb = 4 UMETA(DisplayName = "Do Not Disturb")
};

/**
 * Presence information structure
 */
USTRUCT(BlueprintType, Category = "EOSKit|Presence")
struct EOSKITFRIENDS_API FEOSKitPresenceInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Presence")
	EEOSKitPresenceStatus Status = EEOSKitPresenceStatus::Offline;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Presence")
	FString RichText = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Presence")
	FEOSKitEpicAccountId EpicAccountId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Presence")
	TMap<FString, FString> PresenceData;
};

