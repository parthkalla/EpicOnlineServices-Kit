// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sessions.h"
#include "eos_sessions_types.h"
THIRD_PARTY_INCLUDES_END
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
	TimedOut UMETA(DisplayName = "Timed Out"),
	Cancelled UMETA(DisplayName = "Cancelled"),
	NoConnection UMETA(DisplayName = "No Connection"),
	LimitExceeded UMETA(DisplayName = "Limit Exceeded"),
	PlayerSanctioned UMETA(DisplayName = "Player Sanctioned"),
	Other UMETA(DisplayName = "Other Error")
};

// Helper function to convert EOS_EResult to EEOSKitResult
inline EEOSKitResult ConvertEOSResult(EOS_EResult Result)
{
	switch (Result)
	{
	case EOS_EResult::EOS_Success:
		return EEOSKitResult::Success;
	case EOS_EResult::EOS_InvalidParameters:
		return EEOSKitResult::InvalidParameters;
	case EOS_EResult::EOS_InvalidState:
		return EEOSKitResult::InvalidState;
	case EOS_EResult::EOS_InvalidUser:
		return EEOSKitResult::InvalidUser;
	case EOS_EResult::EOS_NotFound:
		return EEOSKitResult::NotFound;
	case EOS_EResult::EOS_AlreadyExists:
		return EEOSKitResult::AlreadyExists;
	case EOS_EResult::EOS_TimedOut:
		return EEOSKitResult::TimedOut;
	case EOS_EResult::EOS_Cancelled:
		return EEOSKitResult::Cancelled;
	case EOS_EResult::EOS_NoConnection:
		return EEOSKitResult::NoConnection;
	case EOS_EResult::EOS_LimitExceeded:
		return EEOSKitResult::LimitExceeded;
	case EOS_EResult::EOS_Sessions_PlayerSanctioned:
		return EEOSKitResult::PlayerSanctioned;
	default:
		return EEOSKitResult::Other;
	}
}

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

