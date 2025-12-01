// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitLobbySubsystem.h"
#include "EOSKitSubsystem.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

void UEOSKitLobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Lobby Subsystem Initialized"));
}

void UEOSKitLobbySubsystem::Deinitialize()
{
	// Cleanup all notifications
	UnregisterAllNotifications();
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Lobby Subsystem Deinitialized"));
	
	Super::Deinitialize();
}

// ========================================
// Lobby Notifications
// ========================================

bool UEOSKitLobbySubsystem::RegisterJoinLobbyAcceptedNotification(const FEOSKitOnJoinLobbyAccepted& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit Lobby: Cannot register notification - Platform Handle is null"));
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit Lobby: Failed to get Lobby Handle"));
		return false;
	}

	if (JoinLobbyAcceptedNotificationId != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit Lobby: Join lobby accepted notification already registered"));
		return true;
	}

	OnJoinLobbyAcceptedDelegate = Callback;

	EOS_Lobby_AddNotifyJoinLobbyAcceptedOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_ADDNOTIFYJOINLOBBYACCEPTED_API_LATEST;
	
	JoinLobbyAcceptedNotificationId = EOS_Lobby_AddNotifyJoinLobbyAccepted(
		LobbyHandle,
		&Options,
		this,
		[](const EOS_Lobby_JoinLobbyAcceptedCallbackInfo* Data)
		{
			UEOSKitLobbySubsystem::OnJoinLobbyAcceptedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Registered for join lobby accepted notifications"));
	return JoinLobbyAcceptedNotificationId != 0;
}

void UEOSKitLobbySubsystem::UnregisterJoinLobbyAcceptedNotification()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return;
	}

	if (JoinLobbyAcceptedNotificationId != 0)
	{
		EOS_Lobby_RemoveNotifyJoinLobbyAccepted(LobbyHandle, JoinLobbyAcceptedNotificationId);
		JoinLobbyAcceptedNotificationId = 0;
	}
}

bool UEOSKitLobbySubsystem::RegisterLeaveLobbyRequestedNotification(const FEOSKitOnLeaveLobbyRequested& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return false;
	}

	if (LeaveLobbyRequestedNotificationId != 0)
	{
		return true;
	}

	OnLeaveLobbyRequestedDelegate = Callback;

	EOS_Lobby_AddNotifyLeaveLobbyRequestedOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_ADDNOTIFYLEAVELOBBYREQUESTED_API_LATEST;
	
	LeaveLobbyRequestedNotificationId = EOS_Lobby_AddNotifyLeaveLobbyRequested(
		LobbyHandle,
		&Options,
		this,
		[](const EOS_Lobby_LeaveLobbyRequestedCallbackInfo* Data)
		{
			UEOSKitLobbySubsystem::OnLeaveLobbyRequestedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Registered for leave lobby requested notifications"));
	return LeaveLobbyRequestedNotificationId != 0;
}

void UEOSKitLobbySubsystem::UnregisterLeaveLobbyRequestedNotification()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return;
	}

	if (LeaveLobbyRequestedNotificationId != 0)
	{
		EOS_Lobby_RemoveNotifyLeaveLobbyRequested(LobbyHandle, LeaveLobbyRequestedNotificationId);
		LeaveLobbyRequestedNotificationId = 0;
	}
}

bool UEOSKitLobbySubsystem::RegisterLobbyInviteAcceptedNotification(const FEOSKitOnLobbyInviteAccepted& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return false;
	}

	if (LobbyInviteAcceptedNotificationId != 0)
	{
		return true;
	}

	OnLobbyInviteAcceptedDelegate = Callback;

	EOS_Lobby_AddNotifyLobbyInviteAcceptedOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYINVITEACCEPTED_API_LATEST;
	
	LobbyInviteAcceptedNotificationId = EOS_Lobby_AddNotifyLobbyInviteAccepted(
		LobbyHandle,
		&Options,
		this,
		[](const EOS_Lobby_LobbyInviteAcceptedCallbackInfo* Data)
		{
			UEOSKitLobbySubsystem::OnLobbyInviteAcceptedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Registered for lobby invite accepted notifications"));
	return LobbyInviteAcceptedNotificationId != 0;
}

void UEOSKitLobbySubsystem::UnregisterLobbyInviteAcceptedNotification()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return;
	}

	if (LobbyInviteAcceptedNotificationId != 0)
	{
		EOS_Lobby_RemoveNotifyLobbyInviteAccepted(LobbyHandle, LobbyInviteAcceptedNotificationId);
		LobbyInviteAcceptedNotificationId = 0;
	}
}

bool UEOSKitLobbySubsystem::RegisterLobbyInviteReceivedNotification(const FEOSKitOnLobbyInviteReceived& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return false;
	}

	if (LobbyInviteReceivedNotificationId != 0)
	{
		return true;
	}

	OnLobbyInviteReceivedDelegate = Callback;

	EOS_Lobby_AddNotifyLobbyInviteReceivedOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYINVITERECEIVED_API_LATEST;
	
	LobbyInviteReceivedNotificationId = EOS_Lobby_AddNotifyLobbyInviteReceived(
		LobbyHandle,
		&Options,
		this,
		[](const EOS_Lobby_LobbyInviteReceivedCallbackInfo* Data)
		{
			UEOSKitLobbySubsystem::OnLobbyInviteReceivedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Registered for lobby invite received notifications"));
	return LobbyInviteReceivedNotificationId != 0;
}

void UEOSKitLobbySubsystem::UnregisterLobbyInviteReceivedNotification()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return;
	}

	if (LobbyInviteReceivedNotificationId != 0)
	{
		EOS_Lobby_RemoveNotifyLobbyInviteReceived(LobbyHandle, LobbyInviteReceivedNotificationId);
		LobbyInviteReceivedNotificationId = 0;
	}
}

bool UEOSKitLobbySubsystem::RegisterLobbyMemberStatusReceivedNotification(const FEOSKitOnLobbyMemberStatusReceived& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return false;
	}

	if (LobbyMemberStatusReceivedNotificationId != 0)
	{
		return true;
	}

	OnLobbyMemberStatusReceivedDelegate = Callback;

	EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYMEMBERSTATUSRECEIVED_API_LATEST;
	
	LobbyMemberStatusReceivedNotificationId = EOS_Lobby_AddNotifyLobbyMemberStatusReceived(
		LobbyHandle,
		&Options,
		this,
		[](const EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo* Data)
		{
			UEOSKitLobbySubsystem::OnLobbyMemberStatusReceivedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Registered for lobby member status received notifications"));
	return LobbyMemberStatusReceivedNotificationId != 0;
}

void UEOSKitLobbySubsystem::UnregisterLobbyMemberStatusReceivedNotification()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return;
	}

	if (LobbyMemberStatusReceivedNotificationId != 0)
	{
		EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(LobbyHandle, LobbyMemberStatusReceivedNotificationId);
		LobbyMemberStatusReceivedNotificationId = 0;
	}
}

bool UEOSKitLobbySubsystem::RegisterLobbyUpdateReceivedNotification(const FEOSKitOnLobbyUpdateReceived& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return false;
	}

	if (LobbyUpdateReceivedNotificationId != 0)
	{
		return true;
	}

	OnLobbyUpdateReceivedDelegate = Callback;

	EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYUPDATERECEIVED_API_LATEST;
	
	LobbyUpdateReceivedNotificationId = EOS_Lobby_AddNotifyLobbyUpdateReceived(
		LobbyHandle,
		&Options,
		this,
		[](const EOS_Lobby_LobbyUpdateReceivedCallbackInfo* Data)
		{
			UEOSKitLobbySubsystem::OnLobbyUpdateReceivedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Registered for lobby update received notifications"));
	return LobbyUpdateReceivedNotificationId != 0;
}

void UEOSKitLobbySubsystem::UnregisterLobbyUpdateReceivedNotification()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return;
	}

	if (LobbyUpdateReceivedNotificationId != 0)
	{
		EOS_Lobby_RemoveNotifyLobbyUpdateReceived(LobbyHandle, LobbyUpdateReceivedNotificationId);
		LobbyUpdateReceivedNotificationId = 0;
	}
}

bool UEOSKitLobbySubsystem::RegisterLobbyMemberUpdateReceivedNotification(const FEOSKitOnLobbyMemberUpdateReceived& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return false;
	}

	if (LobbyMemberUpdateReceivedNotificationId != 0)
	{
		return true;
	}

	OnLobbyMemberUpdateReceivedDelegate = Callback;

	EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYMEMBERUPDATERECEIVED_API_LATEST;
	
	LobbyMemberUpdateReceivedNotificationId = EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(
		LobbyHandle,
		&Options,
		this,
		[](const EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo* Data)
		{
			UEOSKitLobbySubsystem::OnLobbyMemberUpdateReceivedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Registered for lobby member update received notifications"));
	return LobbyMemberUpdateReceivedNotificationId != 0;
}

void UEOSKitLobbySubsystem::UnregisterLobbyMemberUpdateReceivedNotification()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return;
	}

	if (LobbyMemberUpdateReceivedNotificationId != 0)
	{
		EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(LobbyHandle, LobbyMemberUpdateReceivedNotificationId);
		LobbyMemberUpdateReceivedNotificationId = 0;
	}
}

bool UEOSKitLobbySubsystem::RegisterLobbyInviteRejectedNotification(const FEOSKitOnLobbyInviteRejected& Callback)
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return false;
	}

	if (LobbyInviteRejectedNotificationId != 0)
	{
		return true;
	}

	OnLobbyInviteRejectedDelegate = Callback;

	EOS_Lobby_AddNotifyLobbyInviteRejectedOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYINVITEREJECTED_API_LATEST;
	
	LobbyInviteRejectedNotificationId = EOS_Lobby_AddNotifyLobbyInviteRejected(
		LobbyHandle,
		&Options,
		this,
		[](const EOS_Lobby_LobbyInviteRejectedCallbackInfo* Data)
		{
			UEOSKitLobbySubsystem::OnLobbyInviteRejectedCallback(Data);
		}
	);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Registered for lobby invite rejected notifications"));
	return LobbyInviteRejectedNotificationId != 0;
}

void UEOSKitLobbySubsystem::UnregisterLobbyInviteRejectedNotification()
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	
	if (!LobbyHandle)
	{
		return;
	}

	if (LobbyInviteRejectedNotificationId != 0)
	{
		EOS_Lobby_RemoveNotifyLobbyInviteRejected(LobbyHandle, LobbyInviteRejectedNotificationId);
		LobbyInviteRejectedNotificationId = 0;
	}
}

void UEOSKitLobbySubsystem::UnregisterAllNotifications()
{
	UnregisterJoinLobbyAcceptedNotification();
	UnregisterLeaveLobbyRequestedNotification();
	UnregisterLobbyInviteAcceptedNotification();
	UnregisterLobbyInviteReceivedNotification();
	UnregisterLobbyMemberStatusReceivedNotification();
	UnregisterLobbyUpdateReceivedNotification();
	UnregisterLobbyMemberUpdateReceivedNotification();
	UnregisterLobbyInviteRejectedNotification();
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Unregistered all notifications"));
}

// ========================================
// Private Helper Functions
// ========================================

UEOSKitSubsystem* UEOSKitLobbySubsystem::GetEOSKitSubsystem() const
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

void UEOSKitLobbySubsystem::OnJoinLobbyAcceptedCallback(const void* Data)
{
	const EOS_Lobby_JoinLobbyAcceptedCallbackInfo* CallbackInfo = static_cast<const EOS_Lobby_JoinLobbyAcceptedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitLobbySubsystem* Subsystem = static_cast<UEOSKitLobbySubsystem*>(CallbackInfo->ClientData);
	
	// Convert Product User ID to string
	char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(UserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->LocalUserId, UserIdStr, &BufferSize);
	FString ProductUserId = UTF8_TO_TCHAR(UserIdStr);
	
	FString UiEventId = FString::Printf(TEXT("%llu"), CallbackInfo->UiEventId);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, ProductUserId, UiEventId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Join lobby accepted - User: %s, UI Event: %s"), *ProductUserId, *UiEventId);
			Subsystem->OnJoinLobbyAcceptedDelegate.ExecuteIfBound(ProductUserId, UiEventId);
		}
	});
}

void UEOSKitLobbySubsystem::OnLeaveLobbyRequestedCallback(const void* Data)
{
	const EOS_Lobby_LeaveLobbyRequestedCallbackInfo* CallbackInfo = static_cast<const EOS_Lobby_LeaveLobbyRequestedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitLobbySubsystem* Subsystem = static_cast<UEOSKitLobbySubsystem*>(CallbackInfo->ClientData);
	
	char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(UserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->LocalUserId, UserIdStr, &BufferSize);
	FString ProductUserId = UTF8_TO_TCHAR(UserIdStr);
	
	FString LobbyId = UTF8_TO_TCHAR(CallbackInfo->LobbyId);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, ProductUserId, LobbyId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSKit Lobby: Leave lobby requested - User: %s, Lobby: %s"), *ProductUserId, *LobbyId);
			UE_LOG(LogTemp, Warning, TEXT("EOSKit Lobby: Game must call LeaveLobby - SDK does not automatically leave!"));
			Subsystem->OnLeaveLobbyRequestedDelegate.ExecuteIfBound(ProductUserId, LobbyId);
		}
	});
}

void UEOSKitLobbySubsystem::OnLobbyInviteAcceptedCallback(const void* Data)
{
	const EOS_Lobby_LobbyInviteAcceptedCallbackInfo* CallbackInfo = static_cast<const EOS_Lobby_LobbyInviteAcceptedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitLobbySubsystem* Subsystem = static_cast<UEOSKitLobbySubsystem*>(CallbackInfo->ClientData);
	
	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize1 = sizeof(LocalUserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->LocalUserId, LocalUserIdStr, &BufferSize1);
	FString LocalUserId = UTF8_TO_TCHAR(LocalUserIdStr);
	
	char TargetUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize2 = sizeof(TargetUserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->TargetUserId, TargetUserIdStr, &BufferSize2);
	FString TargetUserId = UTF8_TO_TCHAR(TargetUserIdStr);
	
	FString LobbyId = UTF8_TO_TCHAR(CallbackInfo->LobbyId);
	FString InviteId = UTF8_TO_TCHAR(CallbackInfo->InviteId);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LocalUserId, TargetUserId, LobbyId, InviteId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Invite accepted - Lobby: %s, Invite: %s"), *LobbyId, *InviteId);
			Subsystem->OnLobbyInviteAcceptedDelegate.ExecuteIfBound(LocalUserId, TargetUserId, LobbyId, InviteId);
		}
	});
}

void UEOSKitLobbySubsystem::OnLobbyInviteReceivedCallback(const void* Data)
{
	const EOS_Lobby_LobbyInviteReceivedCallbackInfo* CallbackInfo = static_cast<const EOS_Lobby_LobbyInviteReceivedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitLobbySubsystem* Subsystem = static_cast<UEOSKitLobbySubsystem*>(CallbackInfo->ClientData);
	
	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize1 = sizeof(LocalUserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->LocalUserId, LocalUserIdStr, &BufferSize1);
	FString LocalUserId = UTF8_TO_TCHAR(LocalUserIdStr);
	
	char TargetUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize2 = sizeof(TargetUserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->TargetUserId, TargetUserIdStr, &BufferSize2);
	FString TargetUserId = UTF8_TO_TCHAR(TargetUserIdStr);
	
	FString InviteId = UTF8_TO_TCHAR(CallbackInfo->InviteId);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LocalUserId, TargetUserId, InviteId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Invite received - Invite: %s"), *InviteId);
			Subsystem->OnLobbyInviteReceivedDelegate.ExecuteIfBound(LocalUserId, TargetUserId, InviteId);
		}
	});
}

void UEOSKitLobbySubsystem::OnLobbyMemberStatusReceivedCallback(const void* Data)
{
	const EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo* CallbackInfo = static_cast<const EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitLobbySubsystem* Subsystem = static_cast<UEOSKitLobbySubsystem*>(CallbackInfo->ClientData);
	
	char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(UserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->TargetUserId, UserIdStr, &BufferSize);
	FString TargetUserId = UTF8_TO_TCHAR(UserIdStr);
	
	FString LobbyId = UTF8_TO_TCHAR(CallbackInfo->LobbyId);
	int32 CurrentStatus = static_cast<int32>(CallbackInfo->CurrentStatus);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, TargetUserId, LobbyId, CurrentStatus]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Member status changed - User: %s, Status: %d"), *TargetUserId, CurrentStatus);
			Subsystem->OnLobbyMemberStatusReceivedDelegate.ExecuteIfBound(TargetUserId, LobbyId, CurrentStatus);
		}
	});
}

void UEOSKitLobbySubsystem::OnLobbyUpdateReceivedCallback(const void* Data)
{
	const EOS_Lobby_LobbyUpdateReceivedCallbackInfo* CallbackInfo = static_cast<const EOS_Lobby_LobbyUpdateReceivedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitLobbySubsystem* Subsystem = static_cast<UEOSKitLobbySubsystem*>(CallbackInfo->ClientData);
	
	FString LobbyId = UTF8_TO_TCHAR(CallbackInfo->LobbyId);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LobbyId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Lobby updated - Lobby: %s"), *LobbyId);
			Subsystem->OnLobbyUpdateReceivedDelegate.ExecuteIfBound(LobbyId);
		}
	});
}

void UEOSKitLobbySubsystem::OnLobbyMemberUpdateReceivedCallback(const void* Data)
{
	const EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo* CallbackInfo = static_cast<const EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitLobbySubsystem* Subsystem = static_cast<UEOSKitLobbySubsystem*>(CallbackInfo->ClientData);
	
	FString LobbyId = UTF8_TO_TCHAR(CallbackInfo->LobbyId);
	
	char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize = sizeof(UserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->TargetUserId, UserIdStr, &BufferSize);
	FString TargetUserId = UTF8_TO_TCHAR(UserIdStr);
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LobbyId, TargetUserId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Member updated - Lobby: %s, User: %s"), *LobbyId, *TargetUserId);
			Subsystem->OnLobbyMemberUpdateReceivedDelegate.ExecuteIfBound(LobbyId, TargetUserId);
		}
	});
}

void UEOSKitLobbySubsystem::OnLobbyInviteRejectedCallback(const void* Data)
{
	const EOS_Lobby_LobbyInviteRejectedCallbackInfo* CallbackInfo = static_cast<const EOS_Lobby_LobbyInviteRejectedCallbackInfo*>(Data);
	
	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitLobbySubsystem* Subsystem = static_cast<UEOSKitLobbySubsystem*>(CallbackInfo->ClientData);
	
	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize1 = sizeof(LocalUserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->LocalUserId, LocalUserIdStr, &BufferSize1);
	FString LocalUserId = UTF8_TO_TCHAR(LocalUserIdStr);
	
	char TargetUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferSize2 = sizeof(TargetUserIdStr);
	EOS_ProductUserId_ToString(CallbackInfo->TargetUserId, TargetUserIdStr, &BufferSize2);
	FString TargetUserId = UTF8_TO_TCHAR(TargetUserIdStr);
	
	FString LobbyId = UTF8_TO_TCHAR(CallbackInfo->LobbyId);
	FString InviteId = CallbackInfo->InviteId ? UTF8_TO_TCHAR(CallbackInfo->InviteId) : TEXT("");
	
	AsyncTask(ENamedThreads::GameThread, [Subsystem, LocalUserId, TargetUserId, LobbyId, InviteId]()
	{
		if (Subsystem)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit Lobby: Invite rejected - Local: %s, Target: %s, Lobby: %s, Invite: %s"), 
				*LocalUserId, *TargetUserId, *LobbyId, *InviteId);
			Subsystem->OnLobbyInviteRejectedDelegate.ExecuteIfBound(LocalUserId, TargetUserId, LobbyId, InviteId);
		}
	});
}
