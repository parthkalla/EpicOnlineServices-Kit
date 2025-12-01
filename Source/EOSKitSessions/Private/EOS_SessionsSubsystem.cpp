// Copyright (C) 2024, All Rights Reserved.

#include "EOS_SessionsSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "eos_sessions_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// Helper function to get Sessions Handle
EOS_HSessions UEOS_SessionsSubsystem::GetSessionsHandle()
{
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UGameInstance* GameInstance = Context.World()->GetGameInstance();
				if (UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>())
				{
					if (EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle())
					{
						return EOS_Platform_GetSessionsInterface(PlatformHandle);
					}
				}
			}
		}
	}
	return nullptr;
}

// ========================================
// Active Session Functions
// ========================================

EEOSKitResult UEOS_SessionsSubsystem::EOS_ActiveSession_CopyInfo(const FEOSKitHActiveSession& Handle, FEOSKit_ActiveSession_Info& OutActiveSessionInfo)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Active Session Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Unable to get Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_ActiveSession_CopyInfoOptions Options = {};
	Options.ApiVersion = EOS_ACTIVESESSION_COPYINFO_API_LATEST;
	EOS_ActiveSession_Info* Result = nullptr;
	EOS_EResult ReturnResult = ::EOS_ActiveSession_CopyInfo(Handle.GetEOSHandle(), &Options, &Result);
	
	if (ReturnResult == EOS_EResult::EOS_Success && Result)
	{
		OutActiveSessionInfo.SessionName = Result->SessionName ? UTF8_TO_TCHAR(Result->SessionName) : TEXT("");
		// SessionId is now in SessionDetails
		if (Result->SessionDetails)
		{
			OutActiveSessionInfo.SessionId = Result->SessionDetails->SessionId ? UTF8_TO_TCHAR(Result->SessionDetails->SessionId) : TEXT("");
			OutActiveSessionInfo.NumOpenPublicConnections = Result->SessionDetails->NumOpenPublicConnections;
		}
		else
		{
			OutActiveSessionInfo.SessionId = TEXT("");
			OutActiveSessionInfo.NumOpenPublicConnections = 0;
		}
		OutActiveSessionInfo.LocalUserId = FEOSKitProductUserId(Result->LocalUserId);
		// RegisteredPlayers removed in SDK 1.18 - use EOS_ActiveSession_GetRegisteredPlayerCount/ByIndex instead
		// Get registered player count
		EOS_ActiveSession_GetRegisteredPlayerCountOptions PlayerCountOptions = {};
		PlayerCountOptions.ApiVersion = EOS_ACTIVESESSION_GETREGISTEREDPLAYERCOUNT_API_LATEST;
		OutActiveSessionInfo.RegisteredPlayers = ::EOS_ActiveSession_GetRegisteredPlayerCount(Handle.GetEOSHandle(), &PlayerCountOptions);
		// NumOpenPrivateConnections removed in SDK 1.18
		OutActiveSessionInfo.NumOpenPrivateConnections = 0;
		::EOS_ActiveSession_Info_Release(Result);
	}
	
	return ConvertEOSResult(ReturnResult);
}

FEOSKitProductUserId UEOS_SessionsSubsystem::EOS_ActiveSession_GetRegisteredPlayerByIndex(const FEOSKitHActiveSession& Handle, int32 PlayerIndex)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Active Session Handle"));
		return FEOSKitProductUserId();
	}

	EOS_ActiveSession_GetRegisteredPlayerByIndexOptions Options = {};
	Options.ApiVersion = EOS_ACTIVESESSION_GETREGISTEREDPLAYERBYINDEX_API_LATEST;
	Options.PlayerIndex = PlayerIndex;
	
	EOS_ProductUserId PlayerId = ::EOS_ActiveSession_GetRegisteredPlayerByIndex(Handle.GetEOSHandle(), &Options);
	return FEOSKitProductUserId(PlayerId);
}

void UEOS_SessionsSubsystem::EOS_ActiveSession_Release(const FEOSKitHActiveSession& Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Active Session Handle"));
		return;
	}
	::EOS_ActiveSession_Release(Handle.GetEOSHandle());
}

// ========================================
// Session Details Functions
// ========================================

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionDetails_CopyInfo(const FEOSKitHSessionDetails& Handle, FEOSKit_SessionDetails_Info& OutSessionInfo)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Details Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionDetails_CopyInfoOptions Options = {};
	Options.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;
	EOS_SessionDetails_Info* Result = nullptr;
	EOS_EResult ReturnResult = ::EOS_SessionDetails_CopyInfo(Handle.GetEOSHandle(), &Options, &Result);
	
	if (ReturnResult == EOS_EResult::EOS_Success && Result)
	{
		OutSessionInfo.SessionId = Result->SessionId ? UTF8_TO_TCHAR(Result->SessionId) : TEXT("");
		OutSessionInfo.HostAddress = Result->HostAddress ? UTF8_TO_TCHAR(Result->HostAddress) : TEXT("");
		OutSessionInfo.NumOpenPublicConnections = Result->NumOpenPublicConnections;
		// NumOpenPrivateConnections removed in SDK 1.18
		OutSessionInfo.NumOpenPrivateConnections = 0;
		OutSessionInfo.OwnerUserId = FEOSKitProductUserId(Result->OwnerUserId);
		
		// RegisteredPlayers and RegisteredPlayersCount removed in SDK 1.18
		// To get registered players, use EOS_ActiveSession_GetRegisteredPlayerCount/ByIndex on the active session
		OutSessionInfo.RegisteredPlayers.Empty();
		
		::EOS_SessionDetails_Info_Release(Result);
	}
	
	return ConvertEOSResult(ReturnResult);
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionDetails_CopySessionAttributeByIndex(const FEOSKitHSessionDetails& Handle, int32 AttrIndex, FEOSKit_SessionDetails_Attribute& OutSessionAttribute)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Details Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionDetails_CopySessionAttributeByIndexOptions Options = {};
	Options.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST;
	Options.AttrIndex = AttrIndex;
	EOS_SessionDetails_Attribute* Result = nullptr;
	EOS_EResult ReturnResult = ::EOS_SessionDetails_CopySessionAttributeByIndex(Handle.GetEOSHandle(), &Options, &Result);
	
	if (ReturnResult == EOS_EResult::EOS_Success && Result)
	{
		OutSessionAttribute.Key = Result->Data->Key ? UTF8_TO_TCHAR(Result->Data->Key) : TEXT("");
		
		switch (Result->Data->ValueType)
		{
		case EOS_ESessionAttributeType::EOS_SAT_String:
			OutSessionAttribute.Value = Result->Data->Value.AsUtf8 ? UTF8_TO_TCHAR(Result->Data->Value.AsUtf8) : TEXT("");
			break;
		case EOS_ESessionAttributeType::EOS_SAT_Boolean:
			OutSessionAttribute.BoolValue = Result->Data->Value.AsBool == EOS_TRUE;
			break;
		case EOS_ESessionAttributeType::EOS_SAT_Int64:
			OutSessionAttribute.IntValue = static_cast<int32>(Result->Data->Value.AsInt64);
			break;
		case EOS_ESessionAttributeType::EOS_SAT_Double:
			OutSessionAttribute.DoubleValue = Result->Data->Value.AsDouble;
			break;
		default:
			OutSessionAttribute.Value = TEXT("");
			break;
		}
		
		::EOS_SessionDetails_Attribute_Release(Result);
	}
	
	return ConvertEOSResult(ReturnResult);
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionDetails_CopySessionAttributeByKey(const FEOSKitHSessionDetails& Handle, const FString& AttrKey, FEOSKit_SessionDetails_Attribute& OutSessionAttribute)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Details Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionDetails_CopySessionAttributeByKeyOptions Options = {};
	Options.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYKEY_API_LATEST;
	Options.AttrKey = TCHAR_TO_ANSI(*AttrKey);
	EOS_SessionDetails_Attribute* Result = nullptr;
	EOS_EResult ReturnResult = ::EOS_SessionDetails_CopySessionAttributeByKey(Handle.GetEOSHandle(), &Options, &Result);
	
	if (ReturnResult == EOS_EResult::EOS_Success && Result)
	{
		OutSessionAttribute.Key = Result->Data->Key ? UTF8_TO_TCHAR(Result->Data->Key) : TEXT("");
		
		switch (Result->Data->ValueType)
		{
		case EOS_ESessionAttributeType::EOS_SAT_String:
			OutSessionAttribute.Value = Result->Data->Value.AsUtf8 ? UTF8_TO_TCHAR(Result->Data->Value.AsUtf8) : TEXT("");
			break;
		case EOS_ESessionAttributeType::EOS_SAT_Boolean:
			OutSessionAttribute.BoolValue = Result->Data->Value.AsBool == EOS_TRUE;
			break;
		case EOS_ESessionAttributeType::EOS_SAT_Int64:
			OutSessionAttribute.IntValue = static_cast<int32>(Result->Data->Value.AsInt64);
			break;
		case EOS_ESessionAttributeType::EOS_SAT_Double:
			OutSessionAttribute.DoubleValue = Result->Data->Value.AsDouble;
			break;
		default:
			OutSessionAttribute.Value = TEXT("");
			break;
		}
		
		::EOS_SessionDetails_Attribute_Release(Result);
	}
	
	return ConvertEOSResult(ReturnResult);
}

int32 UEOS_SessionsSubsystem::EOS_SessionDetails_GetSessionAttributeCount(const FEOSKitHSessionDetails& Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Details Handle"));
		return -1;
	}

	EOS_SessionDetails_GetSessionAttributeCountOptions Options = {};
	Options.ApiVersion = EOS_SESSIONDETAILS_GETSESSIONATTRIBUTECOUNT_API_LATEST;
	return ::EOS_SessionDetails_GetSessionAttributeCount(Handle.GetEOSHandle(), &Options);
}

// ========================================
// Session Modification Functions
// ========================================

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_AddAttribute(const FEOSKitHSessionModification& Handle, const FEOSKit_Sessions_AttributeData& AttrData, EEOSKit_SessionAttributeAdvertisementType AdvertisementType)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_AddAttributeOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST;
	Options.AdvertisementType = static_cast<EOS_ESessionAttributeAdvertisementType>(AdvertisementType);
	
	EOS_Sessions_AttributeData LocalTemp = AttrData.GetValueAsEosType();
	Options.SessionAttribute = &LocalTemp;
	
	return ConvertEOSResult(::EOS_SessionModification_AddAttribute(Handle.GetEOSHandle(), &Options));
}

void UEOS_SessionsSubsystem::EOS_SessionModification_Release(const FEOSKitHSessionModification& Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return;
	}
	::EOS_SessionModification_Release(Handle.GetEOSHandle());
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_RemoveAttribute(const FEOSKitHSessionModification& Handle, const FString& Key)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_RemoveAttributeOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_REMOVEATTRIBUTE_API_LATEST;
	Options.Key = TCHAR_TO_ANSI(*Key);
	
	return ConvertEOSResult(::EOS_SessionModification_RemoveAttribute(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_SetAllowedPlatformIds(const FEOSKitHSessionModification& Handle, const TArray<int32>& PlatformIds)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_SetAllowedPlatformIdsOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETALLOWEDPLATFORMIDS_API_LATEST;
	Options.AllowedPlatformIdsCount = PlatformIds.Num();
	
	if (PlatformIds.Num() > 0)
	{
		uint32_t* PlatformIdsData = new uint32_t[PlatformIds.Num()];
		for (int32 i = 0; i < PlatformIds.Num(); i++)
		{
			PlatformIdsData[i] = PlatformIds[i];
		}
		Options.AllowedPlatformIds = PlatformIdsData;
		EOS_EResult Result = ::EOS_SessionModification_SetAllowedPlatformIds(Handle.GetEOSHandle(), &Options);
		delete[] PlatformIdsData;
		return ConvertEOSResult(Result);
	}
	else
	{
		Options.AllowedPlatformIds = nullptr;
		return ConvertEOSResult(::EOS_SessionModification_SetAllowedPlatformIds(Handle.GetEOSHandle(), &Options));
	}
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_SetBucketId(const FEOSKitHSessionModification& Handle, const FString& BucketId)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_SetBucketIdOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETBUCKETID_API_LATEST;
	Options.BucketId = TCHAR_TO_ANSI(*BucketId);
	
	return ConvertEOSResult(::EOS_SessionModification_SetBucketId(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_SetHostAddress(const FEOSKitHSessionModification& Handle, const FString& HostAddress)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_SetHostAddressOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETHOSTADDRESS_API_LATEST;
	Options.HostAddress = TCHAR_TO_ANSI(*HostAddress);
	
	return ConvertEOSResult(::EOS_SessionModification_SetHostAddress(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_SetInvitesAllowed(const FEOSKitHSessionModification& Handle, bool bInvitesAllowed)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_SetInvitesAllowedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETINVITESALLOWED_API_LATEST;
	Options.bInvitesAllowed = bInvitesAllowed ? EOS_TRUE : EOS_FALSE;
	
	return ConvertEOSResult(::EOS_SessionModification_SetInvitesAllowed(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_SetJoinInProgressAllowed(const FEOSKitHSessionModification& Handle, bool bAllowJoinInProgress)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_SetJoinInProgressAllowedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST;
	Options.bAllowJoinInProgress = bAllowJoinInProgress ? EOS_TRUE : EOS_FALSE;
	
	return ConvertEOSResult(::EOS_SessionModification_SetJoinInProgressAllowed(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_SetMaxPlayers(const FEOSKitHSessionModification& Handle, int32 MaxPlayers)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_SetMaxPlayersOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETMAXPLAYERS_API_LATEST;
	Options.MaxPlayers = MaxPlayers;
	
	return ConvertEOSResult(::EOS_SessionModification_SetMaxPlayers(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionModification_SetPermissionLevel(const FEOSKitHSessionModification& Handle, EEOSKit_EOnlineSessionPermissionLevel PermissionLevel)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Modification Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionModification_SetPermissionLevelOptions Options = {};
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
	Options.PermissionLevel = static_cast<EOS_EOnlineSessionPermissionLevel>(PermissionLevel);
	
	return ConvertEOSResult(::EOS_SessionModification_SetPermissionLevel(Handle.GetEOSHandle(), &Options));
}

// ========================================
// Session Notification Functions
// ========================================

FEOSKit_NotificationId UEOS_SessionsSubsystem::EOS_Sessions_AddNotifyJoinSessionAccepted(const FEOSKit_Sessions_OnJoinSessionAcceptedCallback& Callback)
{
	OnJoinSessionAcceptedCallback = Callback;
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return FEOSKit_NotificationId();
	}

	EOS_Sessions_AddNotifyJoinSessionAcceptedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYJOINSESSIONACCEPTED_API_LATEST;
	
	EOS_NotificationId NotificationId = ::EOS_Sessions_AddNotifyJoinSessionAccepted(SessionsHandle, &Options, this, [](const EOS_Sessions_JoinSessionAcceptedCallbackInfo* Data)
	{
		if (UEOS_SessionsSubsystem* Subsystem = static_cast<UEOS_SessionsSubsystem*>(Data->ClientData))
		{
			FEOSKitProductUserId LocalUserId(Data->LocalUserId);
			Subsystem->OnJoinSessionAcceptedCallback.ExecuteIfBound(LocalUserId, static_cast<int64>(Data->UiEventId));
		}
	});
	
	return FEOSKit_NotificationId(NotificationId);
}

FEOSKit_NotificationId UEOS_SessionsSubsystem::EOS_Sessions_AddNotifyLeaveSessionRequested(const FEOSKit_Sessions_OnLeaveSessionRequestedCallback& Callback)
{
	OnLeaveSessionRequestedCallback = Callback;
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return FEOSKit_NotificationId();
	}

	EOS_Sessions_AddNotifyLeaveSessionRequestedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYLEAVESESSIONREQUESTED_API_LATEST;
	
	EOS_NotificationId NotificationId = ::EOS_Sessions_AddNotifyLeaveSessionRequested(SessionsHandle, &Options, this, [](const EOS_Sessions_LeaveSessionRequestedCallbackInfo* Data)
	{
		if (UEOS_SessionsSubsystem* Subsystem = static_cast<UEOS_SessionsSubsystem*>(Data->ClientData))
		{
			FEOSKitProductUserId LocalUserId(Data->LocalUserId);
			FString SessionName = Data->SessionName ? UTF8_TO_TCHAR(Data->SessionName) : TEXT("");
			Subsystem->OnLeaveSessionRequestedCallback.ExecuteIfBound(LocalUserId, SessionName);
		}
	});
	
	return FEOSKit_NotificationId(NotificationId);
}

FEOSKit_NotificationId UEOS_SessionsSubsystem::EOS_Sessions_AddNotifySendSessionNativeInviteRequested(const FEOSKit_Sessions_OnSendSessionInviteCallback& Callback)
{
	OnSendSessionInviteCallback = Callback;
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return FEOSKit_NotificationId();
	}

	EOS_Sessions_AddNotifySendSessionNativeInviteRequestedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYSENDSESSIONNATIVEINVITEREQUESTED_API_LATEST;
	
	EOS_NotificationId NotificationId = ::EOS_Sessions_AddNotifySendSessionNativeInviteRequested(SessionsHandle, &Options, this, [](const EOS_Sessions_SendSessionNativeInviteRequestedCallbackInfo* Data)
	{
		if (UEOS_SessionsSubsystem* Subsystem = static_cast<UEOS_SessionsSubsystem*>(Data->ClientData))
		{
			FEOSKitProductUserId LocalUserId(Data->LocalUserId);
			FString TargetNativeAccountType = Data->TargetNativeAccountType ? UTF8_TO_TCHAR(Data->TargetNativeAccountType) : TEXT("");
			FString TargetUserNativeAccountId = Data->TargetUserNativeAccountId ? UTF8_TO_TCHAR(Data->TargetUserNativeAccountId) : TEXT("");
			FString SessionId = Data->SessionId ? UTF8_TO_TCHAR(Data->SessionId) : TEXT("");
			Subsystem->OnSendSessionInviteCallback.ExecuteIfBound(LocalUserId, static_cast<int64>(Data->UiEventId), TargetNativeAccountType, TargetUserNativeAccountId, SessionId);
		}
	});
	
	return FEOSKit_NotificationId(NotificationId);
}

FEOSKit_NotificationId UEOS_SessionsSubsystem::EOS_Sessions_AddNotifySessionInviteAccepted(const FEOSKit_Sessions_OnSessionInviteAcceptedCallback& Callback)
{
	OnSessionInviteAcceptedCallback = Callback;
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return FEOSKit_NotificationId();
	}

	EOS_Sessions_AddNotifySessionInviteAcceptedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITEACCEPTED_API_LATEST;
	
	EOS_NotificationId NotificationId = ::EOS_Sessions_AddNotifySessionInviteAccepted(SessionsHandle, &Options, this, [](const EOS_Sessions_SessionInviteAcceptedCallbackInfo* Data)
	{
		if (UEOS_SessionsSubsystem* Subsystem = static_cast<UEOS_SessionsSubsystem*>(Data->ClientData))
		{
			FEOSKitProductUserId LocalUserId(Data->LocalUserId);
			FEOSKitProductUserId TargetUserId(Data->TargetUserId);
			FString SessionId = Data->SessionId ? UTF8_TO_TCHAR(Data->SessionId) : TEXT("");
			FString InviteId = Data->InviteId ? UTF8_TO_TCHAR(Data->InviteId) : TEXT("");
			Subsystem->OnSessionInviteAcceptedCallback.ExecuteIfBound(LocalUserId, SessionId, TargetUserId, InviteId);
		}
	});
	
	return FEOSKit_NotificationId(NotificationId);
}

FEOSKit_NotificationId UEOS_SessionsSubsystem::EOS_Sessions_AddNotifySessionInviteReceived(const FEOSKit_Sessions_OnSessionInviteReceivedCallback& Callback)
{
	OnSessionInviteReceivedCallback = Callback;
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return FEOSKit_NotificationId();
	}

	EOS_Sessions_AddNotifySessionInviteReceivedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITERECEIVED_API_LATEST;
	
	EOS_NotificationId NotificationId = ::EOS_Sessions_AddNotifySessionInviteReceived(SessionsHandle, &Options, this, [](const EOS_Sessions_SessionInviteReceivedCallbackInfo* Data)
	{
		if (UEOS_SessionsSubsystem* Subsystem = static_cast<UEOS_SessionsSubsystem*>(Data->ClientData))
		{
			FEOSKitProductUserId LocalUserId(Data->LocalUserId);
			FEOSKitProductUserId TargetUserId(Data->TargetUserId);
			FString InviteId = Data->InviteId ? UTF8_TO_TCHAR(Data->InviteId) : TEXT("");
			Subsystem->OnSessionInviteReceivedCallback.ExecuteIfBound(LocalUserId, InviteId, TargetUserId);
		}
	});
	
	return FEOSKit_NotificationId(NotificationId);
}

FEOSKit_NotificationId UEOS_SessionsSubsystem::EOS_Sessions_AddNotifySessionInviteRejected(const FEOSKit_Sessions_OnSessionInviteRejectedCallback& Callback)
{
	OnSessionInviteRejectedCallback = Callback;
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return FEOSKit_NotificationId();
	}

	EOS_Sessions_AddNotifySessionInviteRejectedOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITEREJECTED_API_LATEST;
	
	EOS_NotificationId NotificationId = ::EOS_Sessions_AddNotifySessionInviteRejected(SessionsHandle, &Options, this, [](const EOS_Sessions_SessionInviteRejectedCallbackInfo* Data)
	{
		if (UEOS_SessionsSubsystem* Subsystem = static_cast<UEOS_SessionsSubsystem*>(Data->ClientData))
		{
			FEOSKitProductUserId LocalUserId(Data->LocalUserId);
			FEOSKitProductUserId TargetUserId(Data->TargetUserId);
			FString SessionId = Data->SessionId ? UTF8_TO_TCHAR(Data->SessionId) : TEXT("");
			FString InviteId = Data->InviteId ? UTF8_TO_TCHAR(Data->InviteId) : TEXT("");
			Subsystem->OnSessionInviteRejectedCallback.ExecuteIfBound(LocalUserId, SessionId, TargetUserId, InviteId);
		}
	});
	
	return FEOSKit_NotificationId(NotificationId);
}

// ========================================
// Session Management Functions
// ========================================

EEOSKitResult UEOS_SessionsSubsystem::EOS_Sessions_CopyActiveSessionHandle(const FString& SessionName, FEOSKitHActiveSession& OutActiveSessionHandle)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_Sessions_CopyActiveSessionHandleOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST;
	Options.SessionName = TCHAR_TO_ANSI(*SessionName);
	
	EOS_HActiveSession LocalHandle = nullptr;
	EOS_EResult ReturnResult = ::EOS_Sessions_CopyActiveSessionHandle(SessionsHandle, &Options, &LocalHandle);
	
	if (ReturnResult == EOS_EResult::EOS_Success)
	{
		OutActiveSessionHandle = FEOSKitHActiveSession(LocalHandle);
	}
	
	return ConvertEOSResult(ReturnResult);
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_Sessions_CopySessionHandleByInviteId(const FString& InviteId, FEOSKitHSessionDetails& OutSessionHandle)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_Sessions_CopySessionHandleByInviteIdOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST;
	Options.InviteId = TCHAR_TO_ANSI(*InviteId);
	
	EOS_HSessionDetails LocalHandle = nullptr;
	EOS_EResult ReturnResult = ::EOS_Sessions_CopySessionHandleByInviteId(SessionsHandle, &Options, &LocalHandle);
	
	if (ReturnResult == EOS_EResult::EOS_Success)
	{
		OutSessionHandle = FEOSKitHSessionDetails(LocalHandle);
	}
	
	return ConvertEOSResult(ReturnResult);
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_Sessions_CopySessionHandleByUiEventId(int64 UiEventId, FEOSKitHSessionDetails& OutSessionHandle)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_Sessions_CopySessionHandleByUiEventIdOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEBYUIEVENTID_API_LATEST;
	Options.UiEventId = static_cast<EOS_UI_EventId>(UiEventId);
	
	EOS_HSessionDetails LocalHandle = nullptr;
	EOS_EResult ReturnResult = ::EOS_Sessions_CopySessionHandleByUiEventId(SessionsHandle, &Options, &LocalHandle);
	
	if (ReturnResult == EOS_EResult::EOS_Success)
	{
		OutSessionHandle = FEOSKitHSessionDetails(LocalHandle);
	}
	
	return ConvertEOSResult(ReturnResult);
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_Sessions_CopySessionHandleForPresence(const FEOSKitProductUserId& LocalUserId, FEOSKitHSessionDetails& OutSessionHandle)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_ProductUserId LocalUserIdEOS = LocalUserId.GetValueAsEosType();
	if (!EOS_ProductUserId_IsValid(LocalUserIdEOS))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Local User ID"));
		return EEOSKitResult::InvalidUser;
	}

	EOS_Sessions_CopySessionHandleForPresenceOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEFORPRESENCE_API_LATEST;
	Options.LocalUserId = LocalUserIdEOS;
	
	EOS_HSessionDetails LocalHandle = nullptr;
	EOS_EResult ReturnResult = ::EOS_Sessions_CopySessionHandleForPresence(SessionsHandle, &Options, &LocalHandle);
	
	if (ReturnResult == EOS_EResult::EOS_Success)
	{
		OutSessionHandle = FEOSKitHSessionDetails(LocalHandle);
	}
	
	return ConvertEOSResult(ReturnResult);
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_Sessions_CreateSessionModification(const FEOSKit_Sessions_CreateSessionModificationOptions& Options, FEOSKitHSessionModification& OutSessionModificationHandle)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_Sessions_CreateSessionModificationOptions LocalOptions = Options.ToEosStruct();
	EOS_HSessionModification LocalHandle = nullptr;
	EOS_EResult ReturnResult = ::EOS_Sessions_CreateSessionModification(SessionsHandle, &LocalOptions, &LocalHandle);
	
	// Clean up allocated memory
	if (LocalOptions.AllowedPlatformIds)
	{
		delete[] LocalOptions.AllowedPlatformIds;
	}
	
	if (ReturnResult == EOS_EResult::EOS_Success)
	{
		OutSessionModificationHandle = FEOSKitHSessionModification(LocalHandle);
	}
	
	return ConvertEOSResult(ReturnResult);
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_Sessions_CreateSessionSearch(int32 MaxSearchResults, FEOSKitHSessionSearch& OutSessionSearchHandle)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_Sessions_CreateSessionSearchOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
	Options.MaxSearchResults = MaxSearchResults;
	
	EOS_HSessionSearch LocalHandle = nullptr;
	EOS_EResult ReturnResult = ::EOS_Sessions_CreateSessionSearch(SessionsHandle, &Options, &LocalHandle);
	
	if (ReturnResult == EOS_EResult::EOS_Success)
	{
		OutSessionSearchHandle = FEOSKitHSessionSearch(LocalHandle);
	}
	
	return ConvertEOSResult(ReturnResult);
}

void UEOS_SessionsSubsystem::EOS_Sessions_DumpSessionState(const FString& SessionName)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return;
	}

	EOS_Sessions_DumpSessionStateOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_DUMPSESSIONSTATE_API_LATEST;
	Options.SessionName = TCHAR_TO_ANSI(*SessionName);
	
	::EOS_Sessions_DumpSessionState(SessionsHandle, &Options);
}

int32 UEOS_SessionsSubsystem::EOS_Sessions_GetInviteCount(const FEOSKitProductUserId& LocalUserId)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return -1;
	}

	EOS_ProductUserId LocalUserIdEOS = LocalUserId.GetValueAsEosType();
	if (!EOS_ProductUserId_IsValid(LocalUserIdEOS))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Local User ID"));
		return -1;
	}

	EOS_Sessions_GetInviteCountOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_GETINVITECOUNT_API_LATEST;
	Options.LocalUserId = LocalUserIdEOS;
	
	return ::EOS_Sessions_GetInviteCount(SessionsHandle, &Options);
}

FString UEOS_SessionsSubsystem::EOS_Sessions_GetInviteIdByIndex(const FEOSKitProductUserId& LocalUserId, int32 Index)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return TEXT("");
	}

	EOS_ProductUserId LocalUserIdEOS = LocalUserId.GetValueAsEosType();
	if (!EOS_ProductUserId_IsValid(LocalUserIdEOS))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Local User ID"));
		return TEXT("");
	}

	EOS_Sessions_GetInviteIdByIndexOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_GETINVITEIDBYINDEX_API_LATEST;
	Options.LocalUserId = LocalUserIdEOS;
	Options.Index = Index;
	
	char Buffer[EOS_SESSIONS_INVITEID_MAX_LENGTH];
	int32_t InOutBufferLength = EOS_SESSIONS_INVITEID_MAX_LENGTH;
	EOS_EResult Result = ::EOS_Sessions_GetInviteIdByIndex(SessionsHandle, &Options, Buffer, &InOutBufferLength);
	
	if (Result == EOS_EResult::EOS_Success)
	{
		return UTF8_TO_TCHAR(Buffer);
	}
	
	return TEXT("");
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_Sessions_IsUserInSession(const FEOSKitProductUserId& TargetUserId, const FString& SessionName)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_ProductUserId TargetUserIdEOS = TargetUserId.GetValueAsEosType();
	if (!EOS_ProductUserId_IsValid(TargetUserIdEOS))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Target User ID"));
		return EEOSKitResult::InvalidUser;
	}

	EOS_Sessions_IsUserInSessionOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_ISUSERINSESSION_API_LATEST;
	Options.TargetUserId = TargetUserIdEOS;
	Options.SessionName = TCHAR_TO_ANSI(*SessionName);
	
	return ConvertEOSResult(::EOS_Sessions_IsUserInSession(SessionsHandle, &Options));
}

void UEOS_SessionsSubsystem::EOS_Sessions_RemoveNotifyJoinSessionAccepted(const FEOSKit_NotificationId& InId)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return;
	}
	::EOS_Sessions_RemoveNotifyJoinSessionAccepted(SessionsHandle, InId.GetEOSNotificationId());
}

void UEOS_SessionsSubsystem::EOS_Sessions_RemoveNotifyLeaveSessionRequested(const FEOSKit_NotificationId& InId)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return;
	}
	::EOS_Sessions_RemoveNotifyLeaveSessionRequested(SessionsHandle, InId.GetEOSNotificationId());
}

void UEOS_SessionsSubsystem::EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested(const FEOSKit_NotificationId& InId)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return;
	}
	::EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested(SessionsHandle, InId.GetEOSNotificationId());
}

void UEOS_SessionsSubsystem::EOS_Sessions_RemoveNotifySessionInviteAccepted(const FEOSKit_NotificationId& InId)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return;
	}
	::EOS_Sessions_RemoveNotifySessionInviteAccepted(SessionsHandle, InId.GetEOSNotificationId());
}

void UEOS_SessionsSubsystem::EOS_Sessions_RemoveNotifySessionInviteReceived(const FEOSKit_NotificationId& InId)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return;
	}
	::EOS_Sessions_RemoveNotifySessionInviteReceived(SessionsHandle, InId.GetEOSNotificationId());
}

void UEOS_SessionsSubsystem::EOS_Sessions_RemoveNotifySessionInviteRejected(const FEOSKit_NotificationId& InId)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return;
	}
	::EOS_Sessions_RemoveNotifySessionInviteRejected(SessionsHandle, InId.GetEOSNotificationId());
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_Sessions_UpdateSessionModification(const FString& SessionName, FEOSKitHSessionModification& OutSessionModificationHandle)
{
	EOS_HSessions SessionsHandle = GetSessionsHandle();
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Sessions Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_Sessions_UpdateSessionModificationOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_UPDATESESSIONMODIFICATION_API_LATEST;
	Options.SessionName = TCHAR_TO_ANSI(*SessionName);
	
	EOS_HSessionModification LocalHandle = nullptr;
	EOS_EResult ReturnResult = ::EOS_Sessions_UpdateSessionModification(SessionsHandle, &Options, &LocalHandle);
	
	if (ReturnResult == EOS_EResult::EOS_Success)
	{
		OutSessionModificationHandle = FEOSKitHSessionModification(LocalHandle);
	}
	
	return ConvertEOSResult(ReturnResult);
}

// ========================================
// Session Search Functions
// ========================================

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionSearch_CopySearchResultByIndex(const FEOSKitHSessionSearch& Handle, int32 SessionIndex, FEOSKitHSessionDetails& OutSessionHandle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Search Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionSearch_CopySearchResultByIndexOptions Options = {};
	Options.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
	Options.SessionIndex = SessionIndex;
	
	EOS_HSessionDetails LocalHandle = nullptr;
	EOS_EResult ReturnResult = ::EOS_SessionSearch_CopySearchResultByIndex(Handle.GetEOSHandle(), &Options, &LocalHandle);
	
	if (ReturnResult == EOS_EResult::EOS_Success)
	{
		OutSessionHandle = FEOSKitHSessionDetails(LocalHandle);
	}
	
	return ConvertEOSResult(ReturnResult);
}

int32 UEOS_SessionsSubsystem::EOS_SessionSearch_GetSearchResultCount(const FEOSKitHSessionSearch& Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Search Handle"));
		return -1;
	}

	EOS_SessionSearch_GetSearchResultCountOptions Options = {};
	Options.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
	
	return ::EOS_SessionSearch_GetSearchResultCount(Handle.GetEOSHandle(), &Options);
}

void UEOS_SessionsSubsystem::EOS_SessionSearch_Release(const FEOSKitHSessionSearch& Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Search Handle"));
		return;
	}
	::EOS_SessionSearch_Release(Handle.GetEOSHandle());
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionSearch_RemoveParameter(const FEOSKitHSessionSearch& Handle, const FString& Key, EEOSKit_EComparisonOp ComparisonOp)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Search Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionSearch_RemoveParameterOptions Options = {};
	Options.ApiVersion = EOS_SESSIONSEARCH_REMOVEPARAMETER_API_LATEST;
	Options.ComparisonOp = static_cast<EOS_EComparisonOp>(ComparisonOp);
	Options.Key = TCHAR_TO_ANSI(*Key);
	
	return ConvertEOSResult(::EOS_SessionSearch_RemoveParameter(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionSearch_SetMaxResults(const FEOSKitHSessionSearch& Handle, int32 MaxSearchResults)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Search Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionSearch_SetMaxResultsOptions Options = {};
	Options.ApiVersion = EOS_SESSIONSEARCH_SETMAXSEARCHRESULTS_API_LATEST;
	Options.MaxSearchResults = MaxSearchResults;
	
	return ConvertEOSResult(::EOS_SessionSearch_SetMaxResults(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionSearch_SetParameter(const FEOSKitHSessionSearch& Handle, const FEOSKit_Sessions_AttributeData& Parameter, EEOSKit_EComparisonOp ComparisonOp)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Search Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionSearch_SetParameterOptions Options = {};
	Options.ApiVersion = EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST;
	Options.ComparisonOp = static_cast<EOS_EComparisonOp>(ComparisonOp);
	
	EOS_Sessions_AttributeData LocalData = Parameter.GetValueAsEosType();
	Options.Parameter = &LocalData;
	
	return ConvertEOSResult(::EOS_SessionSearch_SetParameter(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionSearch_SetSessionId(const FEOSKitHSessionSearch& Handle, const FString& SessionId)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Search Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_SessionSearch_SetSessionIdOptions Options = {};
	Options.ApiVersion = EOS_SESSIONSEARCH_SETSESSIONID_API_LATEST;
	Options.SessionId = TCHAR_TO_ANSI(*SessionId);
	
	return ConvertEOSResult(::EOS_SessionSearch_SetSessionId(Handle.GetEOSHandle(), &Options));
}

EEOSKitResult UEOS_SessionsSubsystem::EOS_SessionSearch_SetTargetUserId(const FEOSKitHSessionSearch& Handle, const FEOSKitProductUserId& TargetUserId)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Session Search Handle"));
		return EEOSKitResult::NotFound;
	}

	EOS_ProductUserId TargetUserIdEOS = TargetUserId.GetValueAsEosType();
	if (!EOS_ProductUserId_IsValid(TargetUserIdEOS))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Target User ID"));
		return EEOSKitResult::InvalidUser;
	}

	EOS_SessionSearch_SetTargetUserIdOptions Options = {};
	Options.ApiVersion = EOS_SESSIONSEARCH_SETTARGETUSERID_API_LATEST;
	Options.TargetUserId = TargetUserIdEOS;
	
	return ConvertEOSResult(::EOS_SessionSearch_SetTargetUserId(Handle.GetEOSHandle(), &Options));
}

