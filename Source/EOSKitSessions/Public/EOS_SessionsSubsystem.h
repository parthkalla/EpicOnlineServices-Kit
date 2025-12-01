// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sessions.h"
#include "eos_sessions_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// .generated.h must always be the last include
#include "EOS_SessionsSubsystem.generated.h"

// ========================================
// Delegates
// ========================================

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKit_Sessions_OnJoinSessionAcceptedCallback, const FEOSKitProductUserId&, LocalUserId, int64, UIEventId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKit_Sessions_OnLeaveSessionRequestedCallback, const FEOSKitProductUserId&, LocalUserId, const FString&, SessionName);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEOSKit_Sessions_OnSendSessionInviteCallback, const FEOSKitProductUserId&, LocalUserId, int64, UIEventId, const FString&, TargetNativeAccountType, const FString&, TargetUserNativeAccountId, const FString&, SessionId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOSKit_Sessions_OnSessionInviteAcceptedCallback, const FEOSKitProductUserId&, LocalUserId, const FString&, SessionId, const FEOSKitProductUserId&, TargetUserId, const FString&, InviteId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOSKit_Sessions_OnSessionInviteReceivedCallback, const FEOSKitProductUserId&, LocalUserId, const FString&, InviteId, const FEOSKitProductUserId&, TargetUserId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOSKit_Sessions_OnSessionInviteRejectedCallback, const FEOSKitProductUserId&, LocalUserId, const FString&, SessionId, const FEOSKitProductUserId&, TargetUserId, const FString&, InviteId);

// ========================================
// Enums
// ========================================

UENUM(BlueprintType)
enum class EEOSKit_SessionAttributeAdvertisementType : uint8
{
	Advertise UMETA(DisplayName = "Advertise"),
	DontAdvertise UMETA(DisplayName = "Don't Advertise")
};

UENUM(BlueprintType)
enum class EEOSKit_EOnlineSessionPermissionLevel : uint8
{
	PublicAdvertised UMETA(DisplayName = "Public Advertised"),
	JoinViaPresence UMETA(DisplayName = "Join Via Presence"),
	InviteOnly UMETA(DisplayName = "Invite Only")
};

UENUM(BlueprintType)
enum class EEOSKit_EComparisonOp : uint8
{
	Equals UMETA(DisplayName = "Equals"),
	NotEquals UMETA(DisplayName = "Not Equals"),
	GreaterThan UMETA(DisplayName = "Greater Than"),
	GreaterThanOrEqual UMETA(DisplayName = "Greater Than Or Equal"),
	LessThan UMETA(DisplayName = "Less Than"),
	LessThanOrEqual UMETA(DisplayName = "Less Than Or Equal"),
	Distance UMETA(DisplayName = "Distance"),
	Near UMETA(DisplayName = "Near")
};

// ========================================
// Structs
// ========================================

USTRUCT(BlueprintType)
struct FEOSKit_Sessions_CreateSessionModificationOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit | Sessions Interface")
	FString SessionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit | Sessions Interface")
	FString BucketId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit | Sessions Interface")
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit | Sessions Interface")
	FEOSKitProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit | Sessions Interface")
	bool bPresenceEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit | Sessions Interface")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit | Sessions Interface")
	bool bSanctionsEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EOSKit | Sessions Interface")
	TArray<int32> AllowedPlatformIds;

	FEOSKit_Sessions_CreateSessionModificationOptions()
		: SessionName(TEXT(""))
		, BucketId(TEXT(""))
		, MaxPlayers(0)
		, LocalUserId()
		, bPresenceEnabled(false)
		, SessionId(TEXT(""))
		, bSanctionsEnabled(false)
	{
	}

	EOS_Sessions_CreateSessionModificationOptions ToEosStruct() const
	{
		EOS_Sessions_CreateSessionModificationOptions EosOptions = {};
		EosOptions.ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST;
		EosOptions.SessionName = TCHAR_TO_ANSI(*SessionName);
		EosOptions.BucketId = TCHAR_TO_ANSI(*BucketId);
		EosOptions.MaxPlayers = MaxPlayers;
		EosOptions.LocalUserId = LocalUserId.GetValueAsEosType();
		EosOptions.bPresenceEnabled = bPresenceEnabled ? EOS_TRUE : EOS_FALSE;
		EosOptions.SessionId = SessionId.IsEmpty() ? nullptr : TCHAR_TO_ANSI(*SessionId);
		EosOptions.bSanctionsEnabled = bSanctionsEnabled ? EOS_TRUE : EOS_FALSE;
		EosOptions.AllowedPlatformIdsCount = AllowedPlatformIds.Num();
		
		if (AllowedPlatformIds.Num() > 0)
		{
			uint32_t* PlatformIdsArray = new uint32_t[AllowedPlatformIds.Num()];
			for (int32 i = 0; i < AllowedPlatformIds.Num(); i++)
			{
				PlatformIdsArray[i] = AllowedPlatformIds[i];
			}
			EosOptions.AllowedPlatformIds = PlatformIdsArray;
		}
		else
		{
			EosOptions.AllowedPlatformIds = nullptr;
		}
		
		return EosOptions;
	}
};

USTRUCT(BlueprintType)
struct FEOSKit_Sessions_AttributeData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString Value;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 IntValue;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	bool BoolValue;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	double DoubleValue;

	FEOSKit_Sessions_AttributeData()
		: Key(TEXT(""))
		, Value(TEXT(""))
		, IntValue(0)
		, BoolValue(false)
		, DoubleValue(0.0)
	{
	}

	EOS_Sessions_AttributeData GetValueAsEosType() const
	{
		EOS_Sessions_AttributeData AttrData = {};
		AttrData.ApiVersion = EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST;
		AttrData.Key = TCHAR_TO_ANSI(*Key);
		
		// Determine value type and set accordingly
		if (!Value.IsEmpty())
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
			AttrData.Value.AsUtf8 = TCHAR_TO_ANSI(*Value);
		}
		else if (BoolValue)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Boolean;
			AttrData.Value.AsBool = EOS_TRUE;
		}
		else if (IntValue != 0)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
			AttrData.Value.AsInt64 = IntValue;
		}
		else if (DoubleValue != 0.0)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Double;
			AttrData.Value.AsDouble = DoubleValue;
		}
		else
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
			AttrData.Value.AsUtf8 = TCHAR_TO_ANSI(*Value);
		}
		
		return AttrData;
	}
};

USTRUCT(BlueprintType)
struct FEOSKit_NotificationId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int64 NotificationIdValue = 0;

	FEOSKit_NotificationId()
		: NotificationIdValue(0)
	{
	}

	FEOSKit_NotificationId(EOS_NotificationId InId)
		: NotificationIdValue(static_cast<int64>(InId))
	{
	}

	EOS_NotificationId GetEOSNotificationId() const
	{
		return static_cast<EOS_NotificationId>(NotificationIdValue);
	}
};

USTRUCT(BlueprintType)
struct FEOSKit_ActiveSession_Info
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString SessionName;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FEOSKitProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 RegisteredPlayers;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 NumOpenPublicConnections;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 NumOpenPrivateConnections;

	FEOSKit_ActiveSession_Info()
		: SessionName(TEXT(""))
		, SessionId(TEXT(""))
		, LocalUserId()
		, RegisteredPlayers(0)
		, NumOpenPublicConnections(0)
		, NumOpenPrivateConnections(0)
	{
	}
};

USTRUCT(BlueprintType)
struct FEOSKit_SessionDetails_Info
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString HostAddress;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 NumOpenPublicConnections;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 NumOpenPrivateConnections;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FEOSKitProductUserId OwnerUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	TArray<FEOSKitProductUserId> RegisteredPlayers;

	FEOSKit_SessionDetails_Info()
		: SessionId(TEXT(""))
		, HostAddress(TEXT(""))
		, NumOpenPublicConnections(0)
		, NumOpenPrivateConnections(0)
		, OwnerUserId()
	{
	}
};

USTRUCT(BlueprintType)
struct FEOSKit_SessionDetails_Attribute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString Value;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 IntValue;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	bool BoolValue;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	double DoubleValue;

	FEOSKit_SessionDetails_Attribute()
		: Key(TEXT(""))
		, Value(TEXT(""))
		, IntValue(0)
		, BoolValue(false)
		, DoubleValue(0.0)
	{
	}
};

// ========================================
// Sessions Subsystem
// ========================================

UCLASS()
class EOSKITSESSIONS_API UEOS_SessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================
	// Active Session Functions
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_ActiveSession_CopyInfo")
	static EEOSKitResult EOS_ActiveSession_CopyInfo(const FEOSKitHActiveSession& Handle, FEOSKit_ActiveSession_Info& OutActiveSessionInfo);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_ActiveSession_GetRegisteredPlayerByIndex")
	static FEOSKitProductUserId EOS_ActiveSession_GetRegisteredPlayerByIndex(const FEOSKitHActiveSession& Handle, int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_ActiveSession_Release")
	static void EOS_ActiveSession_Release(const FEOSKitHActiveSession& Handle);

	// ========================================
	// Session Details Functions
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionDetails_CopyInfo")
	static EEOSKitResult EOS_SessionDetails_CopyInfo(const FEOSKitHSessionDetails& Handle, FEOSKit_SessionDetails_Info& OutSessionInfo);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionDetails_CopySessionAttributeByIndex")
	static EEOSKitResult EOS_SessionDetails_CopySessionAttributeByIndex(const FEOSKitHSessionDetails& Handle, int32 AttrIndex, FEOSKit_SessionDetails_Attribute& OutSessionAttribute);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionDetails_CopySessionAttributeByKey")
	static EEOSKitResult EOS_SessionDetails_CopySessionAttributeByKey(const FEOSKitHSessionDetails& Handle, const FString& AttrKey, FEOSKit_SessionDetails_Attribute& OutSessionAttribute);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionDetails_GetSessionAttributeCount")
	static int32 EOS_SessionDetails_GetSessionAttributeCount(const FEOSKitHSessionDetails& Handle);

	// ========================================
	// Session Modification Functions
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_AddAttribute")
	static EEOSKitResult EOS_SessionModification_AddAttribute(const FEOSKitHSessionModification& Handle, const FEOSKit_Sessions_AttributeData& AttrData, EEOSKit_SessionAttributeAdvertisementType AdvertisementType);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_Release")
	static void EOS_SessionModification_Release(const FEOSKitHSessionModification& Handle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_RemoveAttribute")
	static EEOSKitResult EOS_SessionModification_RemoveAttribute(const FEOSKitHSessionModification& Handle, const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_SetAllowedPlatformIds")
	static EEOSKitResult EOS_SessionModification_SetAllowedPlatformIds(const FEOSKitHSessionModification& Handle, const TArray<int32>& PlatformIds);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_SetBucketId")
	static EEOSKitResult EOS_SessionModification_SetBucketId(const FEOSKitHSessionModification& Handle, const FString& BucketId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_SetHostAddress")
	static EEOSKitResult EOS_SessionModification_SetHostAddress(const FEOSKitHSessionModification& Handle, const FString& HostAddress);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_SetInvitesAllowed")
	static EEOSKitResult EOS_SessionModification_SetInvitesAllowed(const FEOSKitHSessionModification& Handle, bool bInvitesAllowed);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_SetJoinInProgressAllowed")
	static EEOSKitResult EOS_SessionModification_SetJoinInProgressAllowed(const FEOSKitHSessionModification& Handle, bool bAllowJoinInProgress);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_SetMaxPlayers")
	static EEOSKitResult EOS_SessionModification_SetMaxPlayers(const FEOSKitHSessionModification& Handle, int32 MaxPlayers);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionModification_SetPermissionLevel")
	static EEOSKitResult EOS_SessionModification_SetPermissionLevel(const FEOSKitHSessionModification& Handle, EEOSKit_EOnlineSessionPermissionLevel PermissionLevel);

	// ========================================
	// Session Notification Functions
	// ========================================

	FEOSKit_Sessions_OnJoinSessionAcceptedCallback OnJoinSessionAcceptedCallback;
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_AddNotifyJoinSessionAccepted")
	FEOSKit_NotificationId EOS_Sessions_AddNotifyJoinSessionAccepted(const FEOSKit_Sessions_OnJoinSessionAcceptedCallback& Callback);

	FEOSKit_Sessions_OnLeaveSessionRequestedCallback OnLeaveSessionRequestedCallback;
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_AddNotifyLeaveSessionRequested")
	FEOSKit_NotificationId EOS_Sessions_AddNotifyLeaveSessionRequested(const FEOSKit_Sessions_OnLeaveSessionRequestedCallback& Callback);

	FEOSKit_Sessions_OnSendSessionInviteCallback OnSendSessionInviteCallback;
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_AddNotifySendSessionNativeInviteRequested")
	FEOSKit_NotificationId EOS_Sessions_AddNotifySendSessionNativeInviteRequested(const FEOSKit_Sessions_OnSendSessionInviteCallback& Callback);

	FEOSKit_Sessions_OnSessionInviteAcceptedCallback OnSessionInviteAcceptedCallback;
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_AddNotifySessionInviteAccepted")
	FEOSKit_NotificationId EOS_Sessions_AddNotifySessionInviteAccepted(const FEOSKit_Sessions_OnSessionInviteAcceptedCallback& Callback);

	FEOSKit_Sessions_OnSessionInviteReceivedCallback OnSessionInviteReceivedCallback;
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_AddNotifySessionInviteReceived")
	FEOSKit_NotificationId EOS_Sessions_AddNotifySessionInviteReceived(const FEOSKit_Sessions_OnSessionInviteReceivedCallback& Callback);

	FEOSKit_Sessions_OnSessionInviteRejectedCallback OnSessionInviteRejectedCallback;
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_AddNotifySessionInviteRejected")
	FEOSKit_NotificationId EOS_Sessions_AddNotifySessionInviteRejected(const FEOSKit_Sessions_OnSessionInviteRejectedCallback& Callback);

	// ========================================
	// Session Management Functions
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_CopyActiveSessionHandle")
	static EEOSKitResult EOS_Sessions_CopyActiveSessionHandle(const FString& SessionName, FEOSKitHActiveSession& OutActiveSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_CopySessionHandleByInviteId")
	static EEOSKitResult EOS_Sessions_CopySessionHandleByInviteId(const FString& InviteId, FEOSKitHSessionDetails& OutSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_CopySessionHandleByUiEventId")
	static EEOSKitResult EOS_Sessions_CopySessionHandleByUiEventId(int64 UiEventId, FEOSKitHSessionDetails& OutSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_CopySessionHandleForPresence")
	static EEOSKitResult EOS_Sessions_CopySessionHandleForPresence(const FEOSKitProductUserId& LocalUserId, FEOSKitHSessionDetails& OutSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_CreateSessionModification")
	static EEOSKitResult EOS_Sessions_CreateSessionModification(const FEOSKit_Sessions_CreateSessionModificationOptions& Options, FEOSKitHSessionModification& OutSessionModificationHandle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_CreateSessionSearch")
	static EEOSKitResult EOS_Sessions_CreateSessionSearch(int32 MaxSearchResults, FEOSKitHSessionSearch& OutSessionSearchHandle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_DumpSessionState")
	static void EOS_Sessions_DumpSessionState(const FString& SessionName);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_GetInviteCount")
	static int32 EOS_Sessions_GetInviteCount(const FEOSKitProductUserId& LocalUserId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_GetInviteIdByIndex")
	static FString EOS_Sessions_GetInviteIdByIndex(const FEOSKitProductUserId& LocalUserId, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_IsUserInSession")
	static EEOSKitResult EOS_Sessions_IsUserInSession(const FEOSKitProductUserId& TargetUserId, const FString& SessionName);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_RemoveNotifyJoinSessionAccepted")
	static void EOS_Sessions_RemoveNotifyJoinSessionAccepted(const FEOSKit_NotificationId& InId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_RemoveNotifyLeaveSessionRequested")
	static void EOS_Sessions_RemoveNotifyLeaveSessionRequested(const FEOSKit_NotificationId& InId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested")
	static void EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested(const FEOSKit_NotificationId& InId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_RemoveNotifySessionInviteAccepted")
	static void EOS_Sessions_RemoveNotifySessionInviteAccepted(const FEOSKit_NotificationId& InId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_RemoveNotifySessionInviteReceived")
	static void EOS_Sessions_RemoveNotifySessionInviteReceived(const FEOSKit_NotificationId& InId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_RemoveNotifySessionInviteRejected")
	static void EOS_Sessions_RemoveNotifySessionInviteRejected(const FEOSKit_NotificationId& InId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_UpdateSessionModification")
	static EEOSKitResult EOS_Sessions_UpdateSessionModification(const FString& SessionName, FEOSKitHSessionModification& OutSessionModificationHandle);

	// ========================================
	// Session Search Functions
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_CopySearchResultByIndex")
	static EEOSKitResult EOS_SessionSearch_CopySearchResultByIndex(const FEOSKitHSessionSearch& Handle, int32 SessionIndex, FEOSKitHSessionDetails& OutSessionHandle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_GetSearchResultCount")
	static int32 EOS_SessionSearch_GetSearchResultCount(const FEOSKitHSessionSearch& Handle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_Release")
	static void EOS_SessionSearch_Release(const FEOSKitHSessionSearch& Handle);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_RemoveParameter")
	static EEOSKitResult EOS_SessionSearch_RemoveParameter(const FEOSKitHSessionSearch& Handle, const FString& Key, EEOSKit_EComparisonOp ComparisonOp);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_SetMaxResults")
	static EEOSKitResult EOS_SessionSearch_SetMaxResults(const FEOSKitHSessionSearch& Handle, int32 MaxSearchResults);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_SetParameter")
	static EEOSKitResult EOS_SessionSearch_SetParameter(const FEOSKitHSessionSearch& Handle, const FEOSKit_Sessions_AttributeData& Parameter, EEOSKit_EComparisonOp ComparisonOp);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_SetSessionId")
	static EEOSKitResult EOS_SessionSearch_SetSessionId(const FEOSKitHSessionSearch& Handle, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_SetTargetUserId")
	static EEOSKitResult EOS_SessionSearch_SetTargetUserId(const FEOSKitHSessionSearch& Handle, const FEOSKitProductUserId& TargetUserId);

private:
	// Helper function to get Sessions Handle
	static EOS_HSessions GetSessionsHandle();
};

