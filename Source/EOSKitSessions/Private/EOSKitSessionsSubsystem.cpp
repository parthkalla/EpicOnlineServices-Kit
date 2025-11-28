// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitSessionsSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Async/Async.h"

void UEOSKitSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Sessions Subsystem Initialized"));
	
	// Optionally register for notifications automatically
	// RegisterJoinSessionAcceptedNotification();
	// RegisterLeaveSessionRequestedNotification();
	// RegisterSessionInviteReceivedNotification();
}

void UEOSKitSessionsSubsystem::Deinitialize()
{
	// Cleanup all handles and notifications
	UnregisterAllNotifications();
	ClearAllSessionHandles();
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Sessions Subsystem Deinitialized"));
	
	Super::Deinitialize();
}

// ========================================
// Session Handle Management
// ========================================

void UEOSKitSessionsSubsystem::StoreSessionDetailsHandle(const FString& SessionId, int64 Handle)
{
	if (Handle == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit Subsystem: Attempted to store null session handle for %s"), *SessionId);
		return;
	}

	EOS_HSessionDetails SessionHandle = reinterpret_cast<EOS_HSessionDetails>(Handle);
	
	// Release old handle if exists
	if (SessionDetailsHandles.Contains(SessionId))
	{
		EOS_SessionDetails_Release(SessionDetailsHandles[SessionId]);
	}
	
	SessionDetailsHandles.Add(SessionId, SessionHandle);
	UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Stored session handle for %s"), *SessionId);
}

bool UEOSKitSessionsSubsystem::GetSessionDetailsHandle(const FString& SessionId, int64& OutHandle)
{
	if (SessionDetailsHandles.Contains(SessionId))
	{
		OutHandle = reinterpret_cast<int64>(SessionDetailsHandles[SessionId]);
		return true;
	}
	
	OutHandle = 0;
	return false;
}

void UEOSKitSessionsSubsystem::ReleaseSessionDetailsHandle(const FString& SessionId)
{
	if (SessionDetailsHandles.Contains(SessionId))
	{
		EOS_SessionDetails_Release(SessionDetailsHandles[SessionId]);
		SessionDetailsHandles.Remove(SessionId);
		UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Released session handle for %s"), *SessionId);
	}
}

void UEOSKitSessionsSubsystem::ClearAllSessionHandles()
{
	for (auto& Pair : SessionDetailsHandles)
	{
		EOS_SessionDetails_Release(Pair.Value);
	}
	SessionDetailsHandles.Empty();
	UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Cleared all session handles"));
}

// ========================================
// Active Session Management
// ========================================

bool UEOSKitSessionsSubsystem::GetActiveSessionHandle(const FString& SessionName, int64& OutHandle)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return false;
	}

	EOS_Sessions_CopyActiveSessionHandleOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*SessionName);
	Options.SessionName = SessionNameConverter.Get();
	
	EOS_HActiveSession ActiveSessionHandle = nullptr;
	EOS_EResult Result = EOS_Sessions_CopyActiveSessionHandle(SessionsHandle, &Options, &ActiveSessionHandle);
	
	if (Result == EOS_EResult::EOS_Success && ActiveSessionHandle)
	{
		OutHandle = reinterpret_cast<int64>(ActiveSessionHandle);
		return true;
	}
	
	OutHandle = 0;
	return false;
}

// ========================================
// Session Notifications
// ========================================

bool UEOSKitSessionsSubsystem::RegisterJoinSessionAcceptedNotification()
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit Subsystem: Cannot register notification - Sessions handle is null"));
		return false;
	}

	if (JoinSessionAcceptedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit Subsystem: Join session accepted notification already registered"));
		return true;
	}

	EOS_Sessions_AddNotifyJoinSessionAcceptedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYJOINSESSIONACCEPTED_API_LATEST;
	
	JoinSessionAcceptedNotificationId = EOS_Sessions_AddNotifyJoinSessionAccepted(
		SessionsHandle,
		&Options,
		this,
		&UEOSKitSessionsSubsystem::OnJoinSessionAcceptedCallback
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Registered for join session accepted notifications"));
	return JoinSessionAcceptedNotificationId != EOS_INVALID_NOTIFICATIONID;
}

bool UEOSKitSessionsSubsystem::RegisterLeaveSessionRequestedNotification()
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return false;
	}

	if (LeaveSessionRequestedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		return true;
	}

	EOS_Sessions_AddNotifyLeaveSessionRequestedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYLEAVESESSIONREQUESTED_API_LATEST;
	
	LeaveSessionRequestedNotificationId = EOS_Sessions_AddNotifyLeaveSessionRequested(
		SessionsHandle,
		&Options,
		this,
		&UEOSKitSessionsSubsystem::OnLeaveSessionRequestedCallback
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Registered for leave session requested notifications"));
	return LeaveSessionRequestedNotificationId != EOS_INVALID_NOTIFICATIONID;
}

bool UEOSKitSessionsSubsystem::RegisterSessionInviteReceivedNotification()
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return false;
	}

	if (SessionInviteReceivedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		return true;
	}

	EOS_Sessions_AddNotifySessionInviteReceivedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITERECEIVED_API_LATEST;
	
	SessionInviteReceivedNotificationId = EOS_Sessions_AddNotifySessionInviteReceived(
		SessionsHandle,
		&Options,
		this,
		&UEOSKitSessionsSubsystem::OnSessionInviteReceivedCallback
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Registered for session invite received notifications"));
	return SessionInviteReceivedNotificationId != EOS_INVALID_NOTIFICATIONID;
}

bool UEOSKitSessionsSubsystem::RegisterSessionInviteAcceptedNotification()
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return false;
	}

	if (SessionInviteAcceptedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		return true;
	}

	EOS_Sessions_AddNotifySessionInviteAcceptedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITEACCEPTED_API_LATEST;
	
	SessionInviteAcceptedNotificationId = EOS_Sessions_AddNotifySessionInviteAccepted(
		SessionsHandle,
		&Options,
		this,
		&UEOSKitSessionsSubsystem::OnSessionInviteAcceptedCallback
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Registered for session invite accepted notifications"));
	return SessionInviteAcceptedNotificationId != EOS_INVALID_NOTIFICATIONID;
}

bool UEOSKitSessionsSubsystem::RegisterSessionInviteRejectedNotification()
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return false;
	}

	if (SessionInviteRejectedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		return true;
	}

	EOS_Sessions_AddNotifySessionInviteRejectedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITEREJECTED_API_LATEST;
	
	SessionInviteRejectedNotificationId = EOS_Sessions_AddNotifySessionInviteRejected(
		SessionsHandle,
		&Options,
		this,
		&UEOSKitSessionsSubsystem::OnSessionInviteRejectedCallback
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Registered for session invite rejected notifications"));
	return SessionInviteRejectedNotificationId != EOS_INVALID_NOTIFICATIONID;
}

void UEOSKitSessionsSubsystem::UnregisterAllNotifications()
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return;
	}

	if (JoinSessionAcceptedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		EOS_Sessions_RemoveNotifyJoinSessionAccepted(SessionsHandle, JoinSessionAcceptedNotificationId);
		JoinSessionAcceptedNotificationId = EOS_INVALID_NOTIFICATIONID;
	}

	if (LeaveSessionRequestedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		EOS_Sessions_RemoveNotifyLeaveSessionRequested(SessionsHandle, LeaveSessionRequestedNotificationId);
		LeaveSessionRequestedNotificationId = EOS_INVALID_NOTIFICATIONID;
	}

	if (SessionInviteReceivedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		EOS_Sessions_RemoveNotifySessionInviteReceived(SessionsHandle, SessionInviteReceivedNotificationId);
		SessionInviteReceivedNotificationId = EOS_INVALID_NOTIFICATIONID;
	}

	if (SessionInviteAcceptedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		EOS_Sessions_RemoveNotifySessionInviteAccepted(SessionsHandle, SessionInviteAcceptedNotificationId);
		SessionInviteAcceptedNotificationId = EOS_INVALID_NOTIFICATIONID;
	}

	if (SessionInviteRejectedNotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		EOS_Sessions_RemoveNotifySessionInviteRejected(SessionsHandle, SessionInviteRejectedNotificationId);
		SessionInviteRejectedNotificationId = EOS_INVALID_NOTIFICATIONID;
	}

	UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Unregistered all session notifications"));
}

// ========================================
// Session Utility Functions
// ========================================

bool UEOSKitSessionsSubsystem::IsUserInSession(const FString& TargetUserId, const FString& SessionName)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return false;
	}

	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem)
	{
		return false;
	}

	// Note: This is simplified - you'd need proper ProductUserId conversion
	EOS_ProductUserId ProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*TargetUserId));
	if (!ProductUserId || !EOS_ProductUserId_IsValid(ProductUserId))
	{
		return false;
	}

	EOS_Sessions_IsUserInSessionOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ISUSERINSESSION_API_LATEST;
	Options.TargetUserId = ProductUserId;
	
	FTCHARToUTF8 SessionNameConverter(*SessionName);
	Options.SessionName = SessionNameConverter.Get();
	
	EOS_EResult Result = EOS_Sessions_IsUserInSession(SessionsHandle, &Options);
	return Result == EOS_EResult::EOS_Success;
}

int32 UEOSKitSessionsSubsystem::GetInviteCount()
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return 0;
	}

	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem)
	{
		return 0;
	}

	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!LocalUserId)
	{
		return 0;
	}

	EOS_Sessions_GetInviteCountOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_GETINVITECOUNT_API_LATEST;
	Options.LocalUserId = LocalUserId;
	
	return EOS_Sessions_GetInviteCount(SessionsHandle, &Options);
}

FString UEOSKitSessionsSubsystem::GetInviteIdByIndex(int32 Index)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return FString();
	}

	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem)
	{
		return FString();
	}

	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!LocalUserId)
	{
		return FString();
	}

	EOS_Sessions_GetInviteIdByIndexOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_GETINVITEIDBYINDEX_API_LATEST;
	Options.LocalUserId = LocalUserId;
	Options.Index = Index;
	
	char InviteIdBuffer[EOS_SESSIONS_INVITEID_MAX_LENGTH + 1];
	int32_t BufferLength = sizeof(InviteIdBuffer);
	
	EOS_EResult Result = EOS_Sessions_GetInviteIdByIndex(SessionsHandle, &Options, InviteIdBuffer, &BufferLength);
	
	if (Result == EOS_EResult::EOS_Success)
	{
		return UTF8_TO_TCHAR(InviteIdBuffer);
	}
	
	return FString();
}

void UEOSKitSessionsSubsystem::DumpSessionState(const FString& SessionName)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		return;
	}

	EOS_Sessions_DumpSessionStateOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_DUMPSESSIONSTATE_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*SessionName);
	Options.SessionName = SessionNameConverter.Get();
	
	EOS_Sessions_DumpSessionState(SessionsHandle, &Options);
}

// ========================================
// Private Helper Functions
// ========================================

EOS_HSessions UEOSKitSessionsSubsystem::GetSessionsHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}

	return EOS_Platform_GetSessionsInterface(EOSKitSubsystem->GetPlatformHandle());
}

UEOSKitSubsystem* UEOSKitSessionsSubsystem::GetEOSKitSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UEOSKitSubsystem>();
}

// ========================================
// Static Callback Functions
// ========================================

void UEOSKitSessionsSubsystem::OnJoinSessionAcceptedCallback(const EOS_Sessions_JoinSessionAcceptedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSessionsSubsystem* Subsystem = static_cast<UEOSKitSessionsSubsystem*>(Data->ClientData);
	
	// Convert data to strings
	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(LocalUserIdStr);
	EOS_ProductUserId_ToString(Data->LocalUserId, LocalUserIdStr, &BufferSize);
	
	FString LocalUserId = UTF8_TO_TCHAR(LocalUserIdStr);
	FString UiEventId = FString::Printf(TEXT("%llu"), Data->UiEventId);
	
	// Execute on game thread
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LocalUserId, UiEventId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Join session accepted - User: %s, EventId: %s"), *LocalUserId, *UiEventId);
			Subsystem->OnJoinSessionAccepted.Broadcast(LocalUserId, UiEventId);
		}
	});
}

void UEOSKitSessionsSubsystem::OnLeaveSessionRequestedCallback(const EOS_Sessions_LeaveSessionRequestedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSessionsSubsystem* Subsystem = static_cast<UEOSKitSessionsSubsystem*>(Data->ClientData);
	
	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(LocalUserIdStr);
	EOS_ProductUserId_ToString(Data->LocalUserId, LocalUserIdStr, &BufferSize);
	
	FString LocalUserId = UTF8_TO_TCHAR(LocalUserIdStr);
	FString SessionName = UTF8_TO_TCHAR(Data->SessionName);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LocalUserId, SessionName]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Leave session requested - User: %s, Session: %s"), *LocalUserId, *SessionName);
			Subsystem->OnLeaveSessionRequested.Broadcast(LocalUserId, SessionName);
		}
	});
}

void UEOSKitSessionsSubsystem::OnSessionInviteReceivedCallback(const EOS_Sessions_SessionInviteReceivedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSessionsSubsystem* Subsystem = static_cast<UEOSKitSessionsSubsystem*>(Data->ClientData);
	
	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(LocalUserIdStr);
	EOS_ProductUserId_ToString(Data->LocalUserId, LocalUserIdStr, &BufferSize);
	
	char TargetUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	BufferSize = sizeof(TargetUserIdStr);
	EOS_ProductUserId_ToString(Data->TargetUserId, TargetUserIdStr, &BufferSize);
	
	FString LocalUserId = UTF8_TO_TCHAR(LocalUserIdStr);
	FString InviteId = UTF8_TO_TCHAR(Data->InviteId);
	FString TargetUserId = UTF8_TO_TCHAR(TargetUserIdStr);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LocalUserId, InviteId, TargetUserId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Session invite received - InviteId: %s"), *InviteId);
			Subsystem->OnSessionInviteReceived.Broadcast(LocalUserId, InviteId, TargetUserId);
		}
	});
}

void UEOSKitSessionsSubsystem::OnSessionInviteAcceptedCallback(const EOS_Sessions_SessionInviteAcceptedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSessionsSubsystem* Subsystem = static_cast<UEOSKitSessionsSubsystem*>(Data->ClientData);
	
	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(LocalUserIdStr);
	EOS_ProductUserId_ToString(Data->LocalUserId, LocalUserIdStr, &BufferSize);
	
	char TargetUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	BufferSize = sizeof(TargetUserIdStr);
	EOS_ProductUserId_ToString(Data->TargetUserId, TargetUserIdStr, &BufferSize);
	
	FString LocalUserId = UTF8_TO_TCHAR(LocalUserIdStr);
	FString SessionId = UTF8_TO_TCHAR(Data->SessionId);
	FString TargetUserId = UTF8_TO_TCHAR(TargetUserIdStr);
	FString InviteId = UTF8_TO_TCHAR(Data->InviteId);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LocalUserId, SessionId, TargetUserId, InviteId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Session invite accepted - SessionId: %s, InviteId: %s"), *SessionId, *InviteId);
			Subsystem->OnSessionInviteAccepted.Broadcast(LocalUserId, SessionId, TargetUserId, InviteId);
		}
	});
}

void UEOSKitSessionsSubsystem::OnSessionInviteRejectedCallback(const EOS_Sessions_SessionInviteRejectedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSessionsSubsystem* Subsystem = static_cast<UEOSKitSessionsSubsystem*>(Data->ClientData);
	
	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(LocalUserIdStr);
	EOS_ProductUserId_ToString(Data->LocalUserId, LocalUserIdStr, &BufferSize);
	
	char TargetUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	BufferSize = sizeof(TargetUserIdStr);
	EOS_ProductUserId_ToString(Data->TargetUserId, TargetUserIdStr, &BufferSize);
	
	FString LocalUserId = UTF8_TO_TCHAR(LocalUserIdStr);
	FString SessionId = UTF8_TO_TCHAR(Data->SessionId);
	FString TargetUserId = UTF8_TO_TCHAR(TargetUserIdStr);
	FString InviteId = UTF8_TO_TCHAR(Data->InviteId);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LocalUserId, SessionId, TargetUserId, InviteId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Subsystem: Session invite rejected - SessionId: %s, InviteId: %s"), *SessionId, *InviteId);
			Subsystem->OnSessionInviteRejected.Broadcast(LocalUserId, SessionId, TargetUserId, InviteId);
		}
	});
}
