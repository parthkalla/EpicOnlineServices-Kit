// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.generated.h"

/**
 * Blueprint-friendly EOS result wrapper
 */
UENUM(BlueprintType)
enum class EEOSResult : uint8
{
	Success UMETA(DisplayName = "Success"),
	Failed UMETA(DisplayName = "Failed"),
	InvalidParameters UMETA(DisplayName = "Invalid Parameters"),
	NotFound UMETA(DisplayName = "Not Found"),
	AlreadyExists UMETA(DisplayName = "Already Exists"),
	Timeout UMETA(DisplayName = "Timeout"),
	Cancelled UMETA(DisplayName = "Cancelled"),
	InvalidAuth UMETA(DisplayName = "Invalid Auth"),
	InvalidUser UMETA(DisplayName = "Invalid User"),
	InvalidState UMETA(DisplayName = "Invalid State"),
	LimitExceeded UMETA(DisplayName = "Limit Exceeded"),
	Disabled UMETA(DisplayName = "Disabled"),
	NotImplemented UMETA(DisplayName = "Not Implemented"),
	Unknown UMETA(DisplayName = "Unknown Error")
};

/**
 * Safe pointer wrapper for cross-module communication
 */
USTRUCT(BlueprintType)
struct EOSKITSHARED_API FEOSUserPointer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Shared")
	FString UserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Shared")
	int64 PointerValue;

	FEOSUserPointer()
		: UserId(TEXT(""))
		, PointerValue(0)
	{
	}

	FEOSUserPointer(const FString& InUserId, void* InPointer)
		: UserId(InUserId)
		, PointerValue(reinterpret_cast<int64>(InPointer))
	{
	}

	void* GetPointer() const
	{
		return reinterpret_cast<void*>(PointerValue);
	}

	bool IsValid() const
	{
		return PointerValue != 0 && !UserId.IsEmpty();
	}
};

/**
 * EOS connection state
 */
UENUM(BlueprintType)
enum class EEOSConnectionState : uint8
{
	Disconnected UMETA(DisplayName = "Disconnected"),
	Connecting UMETA(DisplayName = "Connecting"),
	Connected UMETA(DisplayName = "Connected"),
	Disconnecting UMETA(DisplayName = "Disconnecting"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Voice chat participant state
 */
UENUM(BlueprintType)
enum class EEOSVoiceParticipantState : uint8
{
	Joined UMETA(DisplayName = "Joined"),
	Left UMETA(DisplayName = "Left"),
	Speaking UMETA(DisplayName = "Speaking"),
	Silent UMETA(DisplayName = "Silent"),
	Muted UMETA(DisplayName = "Muted")
};

/**
 * Common EOS configuration
 */
USTRUCT(BlueprintType)
struct EOSKITSHARED_API FEOSCommonConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Shared")
	int32 TickBudgetInMilliseconds;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Shared")
	bool bEnableLogging;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Shared")
	FString LogDirectory;

	FEOSCommonConfig()
		: TickBudgetInMilliseconds(0)
		, bEnableLogging(true)
		, LogDirectory(TEXT("Saved/Logs"))
	{
	}
};
