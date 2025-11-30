// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitRTCTypes.generated.h"

/**
 * RTC Participant Status enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitRTCParticipantStatus : uint8
{
	Joined = 0		UMETA(DisplayName = "Joined"),
	Left = 1		UMETA(DisplayName = "Left")
};

/**
 * RTC Option (key-value pair)
 */
USTRUCT(BlueprintType, Category = "EOSKit|RTC")
struct EOSKITRTC_API FEOSKitRTCOption
{
	GENERATED_BODY()

	/** The unique key of the option */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	FString Key = TEXT("");

	/** The value of the option */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	FString Value = TEXT("");
};

/**
 * RTC Participant Metadata
 */
USTRUCT(BlueprintType, Category = "EOSKit|RTC")
struct EOSKITRTC_API FEOSKitRTCParticipantMetadata
{
	GENERATED_BODY()

	/** The unique key of this metadata item */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	FString Key = TEXT("");

	/** The value of this metadata item */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	FString Value = TEXT("");
};

/**
 * Participant Status Changed Callback Info
 */
USTRUCT(BlueprintType, Category = "EOSKit|RTC")
struct EOSKITRTC_API FEOSKitRTCParticipantStatusChangedInfo
{
	GENERATED_BODY()

	/** The Product User ID of the local user */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	FEOSKitProductUserId LocalUserId;

	/** The room name */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	FString RoomName = TEXT("");

	/** The participant whose status changed */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	FEOSKitProductUserId ParticipantId;

	/** What status change occurred */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	EEOSKitRTCParticipantStatus ParticipantStatus = EEOSKitRTCParticipantStatus::Left;

	/** The participant metadata items (only set for first Joined notification) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	TArray<FEOSKitRTCParticipantMetadata> ParticipantMetadata;

	/** If true, the participant is in any of the local user's applicable block lists */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|RTC")
	bool bParticipantInBlocklist = false;
};

