// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_common.h"
#include "eos_sessions.h"
#include "eos_sessions_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSessionsSDKShared.generated.h"

// ========================================
// Result Enum (matching EIK pattern)
// ========================================

UENUM(BlueprintType)
enum class EEOSKitResult : uint8
{
	Success UMETA(DisplayName = "Success"),
	InvalidParameters UMETA(DisplayName = "Invalid Parameters"),
	InvalidState UMETA(DisplayName = "Invalid State"),
	InvalidUser UMETA(DisplayName = "Invalid User"),
	NotFound UMETA(DisplayName = "Not Found"),
	AlreadyExists UMETA(DisplayName = "Already Exists"),
	DuplicateNotAllowed UMETA(DisplayName = "Duplicate Not Allowed"),
	TimedOut UMETA(DisplayName = "Timed Out"),
	Canceled UMETA(DisplayName = "Canceled"),
	Cancelled UMETA(DisplayName = "Cancelled"),
	NoConnection UMETA(DisplayName = "No Connection"),
	LimitExceeded UMETA(DisplayName = "Limit Exceeded"),
	PlayerSanctioned UMETA(DisplayName = "Player Sanctioned"),
	Other UMETA(DisplayName = "Other Error")
};

// Helper function to convert EOS_EResult to EEOSKitResult
#if WITH_EOS_SDK
EEOSKitResult ConvertEOSResult(EOS_EResult Result);
#else
EEOSKitResult ConvertEOSResult(int32 Result);
#endif

// ========================================
// Handle Wrappers (for Blueprint compatibility)
// ========================================

USTRUCT(BlueprintType)
struct FEOSKitHSessionDetails
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int64 HandleValue = 0;

	FEOSKitHSessionDetails()
		: HandleValue(0)
	{
	}

	FEOSKitHSessionDetails(EOS_HSessionDetails InHandle)
		: HandleValue(reinterpret_cast<int64>(InHandle))
	{
	}

	EOS_HSessionDetails GetEOSHandle() const
	{
		return reinterpret_cast<EOS_HSessionDetails>(HandleValue);
	}

	bool IsValid() const
	{
		return HandleValue != 0;
	}
};

USTRUCT(BlueprintType)
struct FEOSKitHSessionModification
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int64 HandleValue = 0;

	FEOSKitHSessionModification()
		: HandleValue(0)
	{
	}

	FEOSKitHSessionModification(EOS_HSessionModification InHandle)
		: HandleValue(reinterpret_cast<int64>(InHandle))
	{
	}

	EOS_HSessionModification GetEOSHandle() const
	{
		return reinterpret_cast<EOS_HSessionModification>(HandleValue);
	}

	bool IsValid() const
	{
		return HandleValue != 0;
	}
};

USTRUCT(BlueprintType)
struct FEOSKitHSessionSearch
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int64 HandleValue = 0;

	FEOSKitHSessionSearch()
		: HandleValue(0)
	{
	}

	FEOSKitHSessionSearch(EOS_HSessionSearch InHandle)
		: HandleValue(reinterpret_cast<int64>(InHandle))
	{
	}

	EOS_HSessionSearch GetEOSHandle() const
	{
		return reinterpret_cast<EOS_HSessionSearch>(HandleValue);
	}

	bool IsValid() const
	{
		return HandleValue != 0;
	}
};

USTRUCT(BlueprintType)
struct FEOSKitHActiveSession
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int64 HandleValue = 0;

	FEOSKitHActiveSession()
		: HandleValue(0)
	{
	}

	FEOSKitHActiveSession(EOS_HActiveSession InHandle)
		: HandleValue(reinterpret_cast<int64>(InHandle))
	{
	}

	EOS_HActiveSession GetEOSHandle() const
	{
		return reinterpret_cast<EOS_HActiveSession>(HandleValue);
	}

	bool IsValid() const
	{
		return HandleValue != 0;
	}
};

// FEOSKitProductUserId is now defined in EOSKitSharedTypes.h

