// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CoreGlobals.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/SharedPointer.h"

// EOS SDK includes - wrapped to prevent conflicts
// Must come before .generated.h
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"

#if defined(EOS_PLATFORM_BASE_FILE_NAME)
#include EOS_PLATFORM_BASE_FILE_NAME
#endif
#include "eos_common.h"
#include "eos_types.h"
#include "eos_lobby_types.h"
#include "eos_connect_types.h"
#include "eos_auth_types.h"
#include "eos_userinfo_types.h"
#include "eos_ui_types.h"

#include "Windows/HideWindowsPlatformTypes.h"
#endif

// If the EOS SDK isn't available, provide minimal fallback typedefs so this header still parses
#if !defined(WITH_EOS_SDK) || !WITH_EOS_SDK
#include <cstdint>
typedef const char* EOS_ProductUserId;
typedef const char* EOS_EpicAccountId;
typedef const char* EOS_LobbyId;
typedef int64_t EOS_NotificationId;
typedef int64_t EOS_UI_EventId;
typedef const char* EOS_ContinuanceToken;
typedef int EOS_EResult;
#ifndef EOS_PRODUCTUSERID_MAX_LENGTH
#define EOS_PRODUCTUSERID_MAX_LENGTH 64
#endif
#ifndef EOS_EPICACCOUNTID_MAX_LENGTH
#define EOS_EPICACCOUNTID_MAX_LENGTH 64
#endif
#endif

// .generated.h must always be the last include
#include "EOSKitSharedTypes.generated.h"

// ========================================
// Callback Base Class
// ========================================

/** Used to store a pointer to the EOS callback object without knowing type */
class EOSKITSHARED_API FCallbackBase
{
public:
	virtual ~FCallbackBase() {}
};

#if WITH_EOS_SDK

// ========================================
// Global Callback Template
// ========================================

/** Class to handle all callbacks generically using a lambda to process callback results */
#if ENGINE_MAJOR_VERSION == 5
template<typename CallbackFuncType, typename CallbackParamType, typename OwningType, typename CallbackReturnType = void, typename... CallbackExtraParams>
#else
template<typename CallbackFuncType, typename CallbackType>
#endif
class TEOSKitGlobalCallback :
	public FCallbackBase
{
public:
#if ENGINE_MAJOR_VERSION == 5
	TFunction<CallbackReturnType(const CallbackParamType*, CallbackExtraParams... ExtraParams)> CallbackLambda;
	TEOSKitGlobalCallback(TWeakPtr<OwningType> InOwner)
		: FCallbackBase()
		, Owner(InOwner)
	{
	}
#else
	TFunction<void(const CallbackType*)> CallbackLambda;
	TEOSKitGlobalCallback() = default;
#endif
	virtual ~TEOSKitGlobalCallback() = default;

	CallbackFuncType GetCallbackPtr()
	{
		return &CallbackImpl;
	}

	/** Is this callback intended for the game thread */
	bool bIsGameThreadCallback = true;

private:

#if ENGINE_MAJOR_VERSION == 5

	/** The object that needs to be checked for lifetime before calling the callback */
	TWeakPtr<OwningType> Owner;

	static CallbackReturnType EOS_CALL CallbackImpl(const CallbackParamType* Data, CallbackExtraParams... ExtraParams)
	{
		TEOSKitGlobalCallback* CallbackThis = (TEOSKitGlobalCallback*)Data->ClientData;
		check(CallbackThis);

		if (CallbackThis->bIsGameThreadCallback)
		{
			check(IsInGameThread());
		}

		if (CallbackThis->Owner.IsValid())
		{
			check(CallbackThis->CallbackLambda);

			if constexpr (std::is_void<CallbackReturnType>::value)
			{
				CallbackThis->CallbackLambda(Data, ExtraParams...);
			}
			else
			{
				return CallbackThis->CallbackLambda(Data, ExtraParams...);
			}
		}
		
		if constexpr (!std::is_void<CallbackReturnType>::value)
		{
			// we need to return _something_ to compile.
			return CallbackReturnType{};
		}
	}

#else

	static void EOS_CALL CallbackImpl(const CallbackType* Data)
	{
		check(IsInGameThread());

		TEOSKitGlobalCallback* CallbackThis = (TEOSKitGlobalCallback*)Data->ClientData;
		check(CallbackThis);

		check(CallbackThis->CallbackLambda);
		CallbackThis->CallbackLambda(Data);
	}
#endif
};

#endif // WITH_EOS_SDK

// ========================================
// Comprehensive EOS Result Enum
// ========================================

/**
 * Blueprint-friendly EOS result wrapper
 * Matches EIK's EEIK_Result enum for compatibility
 */
UENUM(BlueprintType)
enum class EEOSResult : uint8
{
	EOS_Success = 0			UMETA(DisplayName = "Success"),
	EOS_NoConnection = 1		UMETA(DisplayName = "No Connection"),
	EOS_InvalidCredentials = 2	UMETA(DisplayName = "Invalid Credentials"),
	EOS_InvalidUser = 3		UMETA(DisplayName = "Invalid User"),
	EOS_InvalidAuth = 4		UMETA(DisplayName = "Invalid Auth"),
	EOS_AccessDenied = 5		UMETA(DisplayName = "Access Denied"),
	EOS_MissingPermissions = 6	UMETA(DisplayName = "Missing Permissions"),
	EOS_Token_Not_Account = 7	UMETA(DisplayName = "Token Not Account"),
	EOS_TooManyRequests = 8		UMETA(DisplayName = "Too Many Requests"),
	EOS_AlreadyPending = 9		UMETA(DisplayName = "Already Pending"),
	EOS_InvalidParameters = 10	UMETA(DisplayName = "Invalid Parameters"),
	EOS_InvalidRequest = 11		UMETA(DisplayName = "Invalid Request"),
	EOS_UnrecognizedResponse = 12	UMETA(DisplayName = "Unrecognized Response"),
	EOS_IncompatibleVersion = 13	UMETA(DisplayName = "Incompatible Version"),
	EOS_NotConfigured = 14		UMETA(DisplayName = "Not Configured"),
	EOS_AlreadyConfigured = 15	UMETA(DisplayName = "Already Configured"),
	EOS_NotImplemented = 16		UMETA(DisplayName = "Not Implemented"),
	EOS_Canceled = 17		UMETA(DisplayName = "Canceled"),
	EOS_NotFound = 18		UMETA(DisplayName = "Not Found"),
	EOS_OperationWillRetry = 19	UMETA(DisplayName = "Operation Will Retry"),
	EOS_NoChange = 20		UMETA(DisplayName = "No Change"),
	EOS_VersionMismatch = 21	UMETA(DisplayName = "Version Mismatch"),
	EOS_LimitExceeded = 22		UMETA(DisplayName = "Limit Exceeded"),
	EOS_Disabled = 23		UMETA(DisplayName = "Disabled"),
	EOS_DuplicateNotAllowed = 24	UMETA(DisplayName = "Duplicate Not Allowed"),
	EOS_InvalidSandboxId = 26	UMETA(DisplayName = "Invalid Sandbox Id"),
	EOS_TimedOut = 27		UMETA(DisplayName = "Timed Out"),
	EOS_PartialResult = 28		UMETA(DisplayName = "Partial Result"),
	EOS_Missing_Role = 29		UMETA(DisplayName = "Missing Role"),
	EOS_Missing_Feature = 30	UMETA(DisplayName = "Missing Feature"),
	EOS_Invalid_Sandbox = 31	UMETA(DisplayName = "Invalid Sandbox"),
	EOS_Invalid_Deployment = 32	UMETA(DisplayName = "Invalid Deployment"),
	EOS_Invalid_Product = 33	UMETA(DisplayName = "Invalid Product"),
	EOS_Invalid_ProductUserID = 34	UMETA(DisplayName = "Invalid Product User ID"),
	EOS_ServiceFailure = 35		UMETA(DisplayName = "Service Failure"),
	EOS_CacheDirectoryMissing = 36	UMETA(DisplayName = "Cache Directory Missing"),
	EOS_CacheDirectoryInvalid = 37	UMETA(DisplayName = "Cache Directory Invalid"),
	EOS_InvalidState = 38		UMETA(DisplayName = "Invalid State"),
	EOS_RequestInProgress = 39	UMETA(DisplayName = "Request In Progress"),
	EOS_ApplicationSuspended = 40	UMETA(DisplayName = "Application Suspended"),
	EOS_NetworkDisconnected = 41	UMETA(DisplayName = "Network Disconnected"),
	EOS_UnexpectedError = 127	UMETA(DisplayName = "Unexpected Error")
};

// ========================================
// Product User ID Wrapper
// ========================================

USTRUCT(BlueprintType)
struct EOSKITSHARED_API FEOSKitProductUserId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit|Connect")
	FString ProductUserId;

private:
#if WITH_EOS_SDK
	EOS_ProductUserId ProductUserIdBasic;
#endif

public:
	FEOSKitProductUserId()
#if WITH_EOS_SDK
		: ProductUserIdBasic(nullptr)
#endif
	{
		ProductUserId = TEXT("");
	}

#if WITH_EOS_SDK
	FEOSKitProductUserId(EOS_ProductUserId InProductUserId)
	{
		ProductUserIdBasic = InProductUserId;
		char ProductIdAnsi[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 ProductIdLen = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
		if (EOS_ProductUserId_IsValid(InProductUserId))
		{
			EOS_ProductUserId_ToString(InProductUserId, ProductIdAnsi, &ProductIdLen);
			ProductUserId = FString(UTF8_TO_TCHAR(ProductIdAnsi));
		}
		else
		{
			ProductUserId = TEXT("");
		}
	}

	EOS_ProductUserId GetValueAsEosType() const
	{
		if (EOS_ProductUserId_IsValid(ProductUserIdBasic))
		{
			return ProductUserIdBasic;
		}
		if (!ProductUserId.IsEmpty())
		{
			EOS_ProductUserId ProductUserIdSec = EOS_ProductUserId_FromString(TCHAR_TO_ANSI(*ProductUserId));
			return ProductUserIdSec;
		}
		return nullptr;
	}

	bool IsValid() const
	{
		return EOS_ProductUserId_IsValid(ProductUserIdBasic) || !ProductUserId.IsEmpty();
	}
#else
	FEOSKitProductUserId(const FString& InProductUserId)
		: ProductUserId(InProductUserId)
	{
	}

	bool IsValid() const
	{
		return !ProductUserId.IsEmpty();
	}
#endif
};

// ========================================
// Epic Account ID Wrapper
// ========================================

USTRUCT(BlueprintType)
struct EOSKITSHARED_API FEOSKitEpicAccountId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit|Auth")
	FString EpicAccountId;

private:
#if WITH_EOS_SDK
	EOS_EpicAccountId Ref;
#endif

public:
	FEOSKitEpicAccountId()
#if WITH_EOS_SDK
		: Ref(nullptr)
#endif
	{
		EpicAccountId = TEXT("");
	}

#if WITH_EOS_SDK
	FEOSKitEpicAccountId(EOS_EpicAccountId InEpicAccountId)
	{
		Ref = InEpicAccountId;

		char AccountIdString[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
		AccountIdString[0] = '\0';
		int32_t BufferSize = sizeof(AccountIdString);
		if (EOS_EpicAccountId_IsValid(InEpicAccountId) == EOS_TRUE &&
			EOS_EpicAccountId_ToString(InEpicAccountId, AccountIdString, &BufferSize) == EOS_EResult::EOS_Success)
		{
			EpicAccountId = UTF8_TO_TCHAR(AccountIdString);
		}
		else
		{
			EpicAccountId = TEXT("");
		}
	}

	EOS_EpicAccountId GetValueAsEosType() const
	{
		if (EOS_EpicAccountId_IsValid(Ref))
		{
			return Ref;
		}
		if (!EpicAccountId.IsEmpty())
		{
			EOS_EpicAccountId EpicAccountIdSec = EOS_EpicAccountId_FromString(TCHAR_TO_ANSI(*EpicAccountId));
			return EpicAccountIdSec;
		}
		return nullptr;
	}

	bool IsValid() const
	{
		return EOS_EpicAccountId_IsValid(Ref) || !EpicAccountId.IsEmpty();
	}
#else
	FEOSKitEpicAccountId(const FString& InEpicAccountId)
		: EpicAccountId(InEpicAccountId)
	{
	}

	bool IsValid() const
	{
		return !EpicAccountId.IsEmpty();
	}
#endif
};

// ========================================
// Lobby ID Wrapper
// ========================================

USTRUCT(BlueprintType)
struct EOSKITSHARED_API FEOSKitLobbyId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit|Lobby")
	FString Value;

private:
#if WITH_EOS_SDK
	EOS_LobbyId Ref;
#endif

public:
	FEOSKitLobbyId()
#if WITH_EOS_SDK
		: Value(TEXT("")), Ref(nullptr)
#else
		: Value(TEXT(""))
#endif
	{
	}

#if WITH_EOS_SDK
	FEOSKitLobbyId(EOS_LobbyId InLobbyId)
	{
		Ref = InLobbyId;
		if (InLobbyId != nullptr)
		{
			Value = UTF8_TO_TCHAR(InLobbyId);
		}
		else
		{
			Value = TEXT("");
		}
	}

	EOS_LobbyId GetValueAsEosType() const
	{
		return Ref;
	}

	bool IsValid() const
	{
		return Ref != nullptr && !Value.IsEmpty();
	}
#else
	FEOSKitLobbyId(const FString& InLobbyId)
		: Value(InLobbyId)
	{
	}

	bool IsValid() const
	{
		return !Value.IsEmpty();
	}
#endif
};

// ========================================
// Notification ID Wrapper
// ========================================

USTRUCT(BlueprintType)
struct EOSKITSHARED_API FEOSKitNotificationId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit|Notifications")
	int64 NotificationId;

#if WITH_EOS_SDK
	FEOSKitNotificationId()
		: NotificationId(0)
	{
	}

	FEOSKitNotificationId(EOS_NotificationId InNotificationId)
		: NotificationId(InNotificationId)
	{
	}

	EOS_NotificationId GetValueAsEosType() const
	{
		return NotificationId;
	}

	bool IsValid() const
	{
		return NotificationId != 0;
	}
#else
	FEOSKitNotificationId()
		: NotificationId(0)
	{
	}

	bool IsValid() const
	{
		return NotificationId != 0;
	}
#endif
};

// ========================================
// UI Event ID Wrapper
// ========================================

USTRUCT(BlueprintType)
struct EOSKITSHARED_API FEOSKitUIEventId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit|UI")
	int64 Value;

private:
#if WITH_EOS_SDK
	EOS_UI_EventId Ref;
#endif

public:
	FEOSKitUIEventId()
#if WITH_EOS_SDK
		: Value(-1), Ref(-1)
#else
		: Value(-1)
#endif
	{
	}

#if WITH_EOS_SDK
	FEOSKitUIEventId(EOS_UI_EventId InEventId)
		: Value(InEventId), Ref(InEventId)
	{
	}

	EOS_UI_EventId GetValueAsEosType() const
	{
		return Ref;
	}

	bool IsValid() const
	{
		return Ref != -1 && Value != -1;
	}
#else
	bool IsValid() const
	{
		return Value != -1;
	}
#endif
};

// ========================================
// Continuance Token Wrapper
// ========================================

USTRUCT(BlueprintType)
struct EOSKITSHARED_API FEOSKitContinuanceToken
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit|Connect")
	FString ContinuanceToken;

private:
#if WITH_EOS_SDK
	EOS_ContinuanceToken ContinuanceTokenBasic;
#endif

public:
	FEOSKitContinuanceToken()
#if WITH_EOS_SDK
		: ContinuanceTokenBasic(nullptr)
#endif
	{
		ContinuanceToken = TEXT("");
	}

#if WITH_EOS_SDK
	FEOSKitContinuanceToken(EOS_ContinuanceToken InContinuanceToken)
	{
		char* OutBuffer = new char[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 OutBufferLen = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
		EOS_ContinuanceToken_ToString(InContinuanceToken, OutBuffer, &OutBufferLen);
		ContinuanceToken = FString(UTF8_TO_TCHAR(OutBuffer));
		delete[] OutBuffer;
		ContinuanceTokenBasic = InContinuanceToken;
	}

	EOS_ContinuanceToken GetValueAsEosType() const
	{
		return ContinuanceTokenBasic;
	}

	bool IsValid() const
	{
		return ContinuanceTokenBasic != nullptr && !ContinuanceToken.IsEmpty();
	}
#else
	FEOSKitContinuanceToken(const FString& InContinuanceToken)
		: ContinuanceToken(InContinuanceToken)
	{
	}

	bool IsValid() const
	{
		return !ContinuanceToken.IsEmpty();
	}
#endif
};

// ========================================
// Existing Types (Preserved)
// ========================================

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

// ========================================
// Helper Functions
// ========================================

#if WITH_EOS_SDK

/**
 * Convert EOS_EResult to EEOSResult enum
 */
EOSKITSHARED_API EEOSResult ConvertEOSResultToEEOSResult(EOS_EResult EosResult);

/**
 * Convert EEOSResult enum to EOS_EResult
 */
EOSKITSHARED_API EOS_EResult ConvertEEOSResultToEOSResult(EEOSResult Result);

#endif // WITH_EOS_SDK
