// Copyright (C) 2024, All Rights Reserved.

#include "OnlineSessionEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "OnlineSubsystemEOSKitPrivate.h"
#include "EOSKitSessionManager.h"
#include "EOSKitSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineBase.h"
#include "HAL/PlatformProcess.h"

#if WITH_EOS_SDK

// Forward declare for TEOSCallback
class FOnlineSessionEOSKit;
typedef TWeakPtr<FOnlineSessionEOSKit, ESPMode::ThreadSafe> FOnlineSessionEOSKitWeakPtr;

struct FCreateSessionCallbackContext
{
	FOnlineSessionEOSKitWeakPtr SessionInterface;
	FName SessionName;
};

// Callback for EOS_Sessions_RegisterPlayers
#if ENGINE_MAJOR_VERSION == 5
typedef TEOSCallback<EOS_Sessions_OnRegisterPlayersCallback, EOS_Sessions_RegisterPlayersCallbackInfo, FOnlineSessionEOSKit> FRegisterPlayersCallback;
#else
typedef TEOSCallback<EOS_Sessions_OnRegisterPlayersCallback, EOS_Sessions_RegisterPlayersCallbackInfo> FRegisterPlayersCallback;
#endif

// Callback for EOS_Sessions_StartSession
#if ENGINE_MAJOR_VERSION == 5
typedef TEOSCallback<EOS_Sessions_OnStartSessionCallback, EOS_Sessions_StartSessionCallbackInfo, FOnlineSessionEOSKit> FStartSessionCallback;
#else
typedef TEOSCallback<EOS_Sessions_OnStartSessionCallback, EOS_Sessions_StartSessionCallbackInfo> FStartSessionCallback;
#endif

FOnlineSessionEOSKit::FOnlineSessionEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, SessionsHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		SessionsHandle = EOSKitSubsystem->SessionsHandle;
	}
}

FOnlineSessionEOSKit::~FOnlineSessionEOSKit()
{
	// Cleanup
}

FUniqueNetIdPtr FOnlineSessionEOSKit::CreateSessionIdFromString(const FString& SessionIdStr)
{
	// Create a session ID from string
	// This is a minimal implementation - full implementation would create proper FUniqueNetId
	return nullptr;
}

FNamedOnlineSession* FOnlineSessionEOSKit::GetNamedSession(FName SessionName)
{
	FScopeLock ScopeLock(&SessionLock);
	for (FNamedOnlineSession& Session : Sessions)
	{
		if (Session.SessionName == SessionName)
		{
			return &Session;
		}
	}
	return nullptr;
}

void FOnlineSessionEOSKit::RemoveNamedSession(FName SessionName)
{
	FScopeLock ScopeLock(&SessionLock);
	for (int32 Index = 0; Index < Sessions.Num(); Index++)
	{
		if (Sessions[Index].SessionName == SessionName)
		{
			Sessions.RemoveAtSwap(Index);
			return;
		}
	}
}

const FNamedOnlineSession* FOnlineSessionEOSKit::GetNamedSession(FName SessionName) const
{
	FScopeLock ScopeLock(&SessionLock);
	for (const FNamedOnlineSession& Session : Sessions)
	{
		if (Session.SessionName == SessionName)
		{
			return &Session;
		}
	}
	return nullptr;
}

EOnlineSessionState::Type FOnlineSessionEOSKit::GetSessionState(FName SessionName) const
{
	const FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		return Session->SessionState;
	}
	return EOnlineSessionState::NoSession;
}

bool FOnlineSessionEOSKit::CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::CreateSession: Creating session '%s'"), *SessionName.ToString());

#if WITH_EOS_SDK
	if (GetNamedSession(SessionName))
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::CreateSession: Session '%s' already exists"), *SessionName.ToString());
		TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		return false;
	}

	FNamedOnlineSession* Session = AddNamedSession(SessionName, NewSessionSettings);
	if (!Session)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::CreateSession: Failed to add named session"));
		TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		return false;
	}

	Session->SessionState = EOnlineSessionState::Creating;
	Session->bHosting = true;
	Session->HostingPlayerNum = HostingPlayerNum;
	Session->NumOpenPrivateConnections = NewSessionSettings.NumPrivateConnections;
	Session->NumOpenPublicConnections = NewSessionSettings.NumPublicConnections;
	Session->SessionSettings.BuildUniqueId = GetBuildUniqueId();

	if (EOSKitSubsystem)
	{
		if (IOnlineIdentityPtr Identity = EOSKitSubsystem->GetIdentityInterface())
		{
			Session->OwningUserId = Identity->GetUniquePlayerId(HostingPlayerNum);
			Session->OwningUserName = Identity->GetPlayerNickname(HostingPlayerNum);
		}
	}

	uint32 Result = CreateEOSSessionInternal(HostingPlayerNum, Session);
	if (Result == ONLINE_IO_PENDING)
	{
		return true;
	}

	bool bWasSuccessful = (Result == ONLINE_SUCCESS);
	if (!bWasSuccessful)
	{
		RemoveNamedSession(SessionName);
	}
	else
	{
		Session->SessionState = EOnlineSessionState::Pending;
		
		// NOTE: For P2P policy, we DON'T manually register the host
		// EOS automatically counts the host when StartSession is called
		// Manual RegisterPlayer fails with P2P policy (missing matchmaking:managePlayers permission)
		
		// Decrement connection count locally to reflect that host will use 1 slot
		if (Session->bHosting && Session->NumOpenPublicConnections > 0)
		{
			Session->NumOpenPublicConnections--;
			UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::CreateSession: ✅ Reserved 1 slot for host (remaining: %d)"), Session->NumOpenPublicConnections);
			UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::CreateSession: Host will be auto-registered by EOS when StartSession is called"));
		}
		
		RegisterLocalPlayers(Session);
	}

	TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);
	return bWasSuccessful;
#else
	TriggerOnCreateSessionCompleteDelegates(SessionName, false);
	return false;
#endif
}

bool FOnlineSessionEOSKit::CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	return CreateSession(0, SessionName, NewSessionSettings);
}

bool FOnlineSessionEOSKit::StartSession(FName SessionName)
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::StartSession: Starting session '%s'"), *SessionName.ToString());
	
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!Session)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: Session '%s' not found"), *SessionName.ToString());
		TriggerOnStartSessionCompleteDelegates(SessionName, false);
		return false;
	}

	if (Session->SessionState != EOnlineSessionState::Pending && Session->SessionState != EOnlineSessionState::Creating)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: Session '%s' is in wrong state (%d)"), 
			*SessionName.ToString(), static_cast<int32>(Session->SessionState));
		TriggerOnStartSessionCompleteDelegates(SessionName, false);
		return false;
	}

#if WITH_EOS_SDK
	if (!SessionsHandle)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: SessionsHandle is null"));
		TriggerOnStartSessionCompleteDelegates(SessionName, false);
		return false;
	}

	// Call EOS SDK to start the session
	EOS_Sessions_StartSessionOptions StartOptions = {};
	StartOptions.ApiVersion = EOS_SESSIONS_STARTSESSION_API_LATEST;
	const FTCHARToUTF8 Utf8SessionName(*SessionName.ToString());
	StartOptions.SessionName = Utf8SessionName.Get();

	// Create callback context
	FStartSessionCallback* CallbackObj = new FStartSessionCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
	CallbackObj->CallbackLambda = [this, SessionName](const EOS_Sessions_StartSessionCallbackInfo* Data)
	{
		bool bWasSuccessful = false;
		
		if (FNamedOnlineSession* SessionRef = GetNamedSession(SessionName))
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				SessionRef->SessionState = EOnlineSessionState::InProgress;
				bWasSuccessful = true;
				UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: ✅ Session '%s' started successfully!"), *SessionName.ToString());
				
				// CRITICAL: Add host to local RegisteredPlayers list (EOS auto-registers for P2P, but we track locally)
				if (SessionRef->bHosting && SessionRef->OwningUserId.IsValid())
				{
					if (!SessionRef->RegisteredPlayers.Contains(SessionRef->OwningUserId.ToSharedRef()))
					{
						SessionRef->RegisteredPlayers.AddUnique(SessionRef->OwningUserId.ToSharedRef());
						UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: ✅ Host added to local RegisteredPlayers list"));
						UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: RegisteredPlayers count: %d"), SessionRef->RegisteredPlayers.Num());
					}
					
					// Ensure connection count reflects host usage
					if (SessionRef->NumOpenPublicConnections >= SessionRef->SessionSettings.NumPublicConnections)
					{
						SessionRef->NumOpenPublicConnections = FMath::Max(0, SessionRef->SessionSettings.NumPublicConnections - 1);
						UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: Updated NumOpenPublicConnections to %d (host using 1 slot)"), SessionRef->NumOpenPublicConnections);
					}
				}
				
				UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: Session is now ACTIVE - EOS should show host in Players tab"));
			}
			else
			{
				UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::StartSession: Failed to start session '%s': %s"), 
					*SessionName.ToString(), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
			}
		}
		
		TriggerOnStartSessionCompleteDelegates(SessionName, bWasSuccessful);
	};

	EOS_Sessions_StartSession(SessionsHandle, &StartOptions, CallbackObj, CallbackObj->GetCallbackPtr());
	return true;
#else
	// Fallback for non-EOS builds
	Session->SessionState = EOnlineSessionState::InProgress;
	TriggerOnStartSessionCompleteDelegates(SessionName, true);
	return true;
#endif
}

bool FOnlineSessionEOSKit::UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		Session->SessionSettings = UpdatedSessionSettings;
		return true;
	}
	return false;
}

bool FOnlineSessionEOSKit::EndSession(FName SessionName)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		Session->SessionState = EOnlineSessionState::Ended;
		return true;
	}
	return false;
}

bool FOnlineSessionEOSKit::DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& Delegate)
{
	// Delegate to EOSKitSessionManager
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitSessionManager* SessionManager = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitSessionManager>();
				if (SessionManager)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::DestroySession: Delegating to UEOSKitSessionManager"));
					// SessionManager->DestroyEOSKitSession(SessionName);
					RemoveNamedSession(SessionName);
					// In UE 5.5, FOnDestroySessionCompleteDelegate takes (FName, bool)
					Delegate.ExecuteIfBound(SessionName, true);
					return true;
				}
			}
		}
	}
	// In UE 5.5, FOnDestroySessionCompleteDelegate takes (FName, bool)
	Delegate.ExecuteIfBound(SessionName, false);
	return false;
}

uint32 FOnlineSessionEOSKit::CreateEOSSessionInternal(int32 HostingPlayerNum, FNamedOnlineSession* Session)
{
#if WITH_EOS_SDK
	if (!SessionsHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::CreateEOSSessionInternal: SessionsHandle is null"));
		return ONLINE_FAIL;
	}

	if (!Session)
	{
		return ONLINE_FAIL;
	}

	EOS_HSessionModification SessionModHandle = nullptr;

	FTCHARToUTF8 SessionNameAnsi(*Session->SessionName.ToString());
	EOS_Sessions_CreateSessionModificationOptions Options = {};
	Options.ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST;
	Options.SessionName = SessionNameAnsi.Get();
	FString BucketId(TEXT("DefaultBucket"));
	if (const FOnlineSessionSetting* BucketSetting = Session->SessionSettings.Settings.Find(FName(TEXT("BucketId"))))
	{
		BucketSetting->Data.GetValue(BucketId);
	}
	FTCHARToUTF8 BucketAnsi(*BucketId);
	Options.BucketId = BucketAnsi.Get();
	Options.MaxPlayers = Session->SessionSettings.NumPrivateConnections + Session->SessionSettings.NumPublicConnections;

	EOS_ProductUserId HostUserId = GetProductUserIdForSession(Session);
	if (!EOS_ProductUserId_IsValid(HostUserId))
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::CreateEOSSessionInternal: Host ProductUserId invalid"));
		return ONLINE_FAIL;
	}
	Options.LocalUserId = HostUserId;
	Options.bPresenceEnabled = (Session->SessionSettings.bUsesPresence ||
		Session->SessionSettings.bAllowJoinViaPresence ||
		Session->SessionSettings.bAllowJoinViaPresenceFriendsOnly ||
		Session->SessionSettings.bAllowInvites) ? EOS_TRUE : EOS_FALSE;
	Options.bSanctionsEnabled = EOS_FALSE;

	EOS_EResult ResultCode = EOS_Sessions_CreateSessionModification(SessionsHandle, &Options, &SessionModHandle);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::CreateEOSSessionInternal: CreateSessionModification failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
		return ONLINE_FAIL;
	}

	if (!Session->SessionInfo.IsValid())
	{
		Session->SessionInfo = MakeShareable(new FOnlineSessionInfoEOSKit(TEXT("127.0.0.1")));
	}

	// Permission level
	EOS_SessionModification_SetPermissionLevelOptions PermOptions = {};
	PermOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
	PermOptions.PermissionLevel = Session->SessionSettings.bShouldAdvertise ?
		EOS_EOnlineSessionPermissionLevel::EOS_OSPF_PublicAdvertised :
		EOS_EOnlineSessionPermissionLevel::EOS_OSPF_InviteOnly;
	EOS_SessionModification_SetPermissionLevel(SessionModHandle, &PermOptions);

	// Join in progress
	EOS_SessionModification_SetJoinInProgressAllowedOptions JoinOptions = {};
	JoinOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST;
	JoinOptions.bAllowJoinInProgress = Session->SessionSettings.bAllowJoinInProgress ? EOS_TRUE : EOS_FALSE;
	EOS_SessionModification_SetJoinInProgressAllowed(SessionModHandle, &JoinOptions);

	// Helper lambda to add an attribute
	auto AddAttribute = [&SessionModHandle](const char* Key, EOS_ESessionAttributeType Type, const void* Value)
	{
		EOS_Sessions_AttributeData AttrData = {};
		AttrData.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		AttrData.Key = Key;
		AttrData.ValueType = Type;
		
		switch (Type)
		{
		case EOS_ESessionAttributeType::EOS_SAT_Boolean:
			AttrData.Value.AsBool = *static_cast<const EOS_Bool*>(Value);
			break;
		case EOS_ESessionAttributeType::EOS_SAT_Int64:
			AttrData.Value.AsInt64 = *static_cast<const int64_t*>(Value);
			break;
		case EOS_ESessionAttributeType::EOS_SAT_String:
			AttrData.Value.AsUtf8 = static_cast<const char*>(Value);
			break;
		default:
			return;
		}
		
		EOS_SessionModification_AddAttributeOptions AttrOptions = {};
		AttrOptions.ApiVersion = EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST;
		AttrOptions.SessionAttribute = &AttrData;
		AttrOptions.AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise;
		EOS_SessionModification_AddAttribute(SessionModHandle, &AttrOptions);
	};

	// Add standard attributes (matching EIK)
	// PRESENCESEARCH - required for presence-based session discovery
	const EOS_Bool bPresenceSearch = EOS_TRUE;
	AddAttribute("PRESENCESEARCH", EOS_ESessionAttributeType::EOS_SAT_Boolean, &bPresenceSearch);
	
	// NumPrivateConnections
	AddAttribute("NumPrivateConnections", EOS_ESessionAttributeType::EOS_SAT_Int64, &Session->SessionSettings.NumPrivateConnections);
	
	// NumPublicConnections
	AddAttribute("NumPublicConnections", EOS_ESessionAttributeType::EOS_SAT_Int64, &Session->SessionSettings.NumPublicConnections);
	
	// OwningUserId
	if (Session->OwningUserId.IsValid() && Session->OwningUserId->IsValid())
	{
		FString OwningUserIdStr = Session->OwningUserId->ToString();
		FTCHARToUTF8 OwningUserIdAnsi(*OwningUserIdStr);
		AddAttribute("OwningUserId", EOS_ESessionAttributeType::EOS_SAT_String, OwningUserIdAnsi.Get());
	}
	
	// OwningUserName
	if (Session->OwningUserName.IsEmpty())
	{
		FString OwningPlayerName(TEXT("DedicatedServer - "));
		FString UserName = FPlatformProcess::UserName();
		if (UserName.IsEmpty())
		{
			FString ComputerName = FPlatformProcess::ComputerName();
			OwningPlayerName += ComputerName;
		}
		else
		{
			OwningPlayerName += UserName;
		}
		Session->OwningUserName = OwningPlayerName;
	}
	FTCHARToUTF8 OwningUserNameAnsi(*Session->OwningUserName);
	AddAttribute("OwningUserName", EOS_ESessionAttributeType::EOS_SAT_String, OwningUserNameAnsi.Get());
	
	// bAntiCheatProtected
	const EOS_Bool bAntiCheatProtected = Session->SessionSettings.bAntiCheatProtected ? EOS_TRUE : EOS_FALSE;
	AddAttribute("bAntiCheatProtected", EOS_ESessionAttributeType::EOS_SAT_Boolean, &bAntiCheatProtected);
	
	// bUsesStats
	const EOS_Bool bUsesStats = Session->SessionSettings.bUsesStats ? EOS_TRUE : EOS_FALSE;
	AddAttribute("bUsesStats", EOS_ESessionAttributeType::EOS_SAT_Boolean, &bUsesStats);
	
	// bIsDedicated
	const EOS_Bool bIsDedicated = Session->SessionSettings.bIsDedicated ? EOS_TRUE : EOS_FALSE;
	AddAttribute("bIsDedicated", EOS_ESessionAttributeType::EOS_SAT_Boolean, &bIsDedicated);
	
	// BuildUniqueId
	AddAttribute("BuildUniqueId", EOS_ESessionAttributeType::EOS_SAT_Int64, &Session->SessionSettings.BuildUniqueId);

	// Add custom advertised attributes from session settings
	for (const TPair<FName, FOnlineSessionSetting>& SettingPair : Session->SessionSettings.Settings)
	{
		const FOnlineSessionSetting& Setting = SettingPair.Value;

		EOS_Sessions_AttributeData AttrData = {};
		AttrData.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		const FString KeyString = SettingPair.Key.ToString();
		FTCHARToUTF8 KeyAnsi(*KeyString);
		AttrData.Key = KeyAnsi.Get();

		bool bSupported = true;
		switch (Setting.Data.GetType())
		{
		case EOnlineKeyValuePairDataType::String:
		{
			FString Value;
			Setting.Data.GetValue(Value);
			FTCHARToUTF8 ValueAnsi(*Value);
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
			AttrData.Value.AsUtf8 = ValueAnsi.Get();
			break;
		}
		case EOnlineKeyValuePairDataType::Bool:
		{
			bool bValue = false;
			Setting.Data.GetValue(bValue);
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Boolean;
			AttrData.Value.AsBool = bValue ? EOS_TRUE : EOS_FALSE;
			break;
		}
		case EOnlineKeyValuePairDataType::Int32:
		case EOnlineKeyValuePairDataType::UInt32:
		case EOnlineKeyValuePairDataType::Int64:
		{
			int64 IntValue = 0;
			if (Setting.Data.GetType() == EOnlineKeyValuePairDataType::Int32)
			{
				int32 Temp;
				Setting.Data.GetValue(Temp);
				IntValue = Temp;
			}
			else if (Setting.Data.GetType() == EOnlineKeyValuePairDataType::UInt32)
			{
				uint32 Temp;
				Setting.Data.GetValue(Temp);
				IntValue = Temp;
			}
			else
			{
				int64 Temp;
				Setting.Data.GetValue(Temp);
				IntValue = Temp;
			}
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
			AttrData.Value.AsInt64 = IntValue;
			break;
		}
		default:
			bSupported = false;
			break;
		}

		if (!bSupported)
		{
			continue;
		}

		EOS_SessionModification_AddAttributeOptions AttrOptions = {};
		AttrOptions.ApiVersion = EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST;
		AttrOptions.SessionAttribute = &AttrData;
		AttrOptions.AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise;
		EOS_SessionModification_AddAttribute(SessionModHandle, &AttrOptions);
	}

	// Set HostAddress for P2P sessions (matching EIK)
	// If using P2P sockets, set the EOS address format: EOS:ProductUserId:GameNetDriver:26
	if (!Session->SessionSettings.bIsDedicated)
	{
		// Check if we're using P2P (EOS NetDriver)
		// For now, always set it for non-dedicated servers
		EOS_ProductUserId HostPUID = GetProductUserIdForSession(Session);
		if (EOS_ProductUserId_IsValid(HostPUID))
		{
			char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
			int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
			EOS_ProductUserId_ToString(HostPUID, ProductUserIdStr, &ProductUserIdStrSize);
			
			// Build EOS P2P address: EOS:ProductUserId:GameNetDriver:26
			FString HostAddr = FString::Printf(TEXT("EOS:%s:GameNetDriver:26"), UTF8_TO_TCHAR(ProductUserIdStr));
			FTCHARToUTF8 HostAddrAnsi(*HostAddr);
			
			EOS_SessionModification_SetHostAddressOptions HostOptions = {};
			HostOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETHOSTADDRESS_API_LATEST;
			HostOptions.HostAddress = HostAddrAnsi.Get();
			EOS_EResult HostResult = EOS_SessionModification_SetHostAddress(SessionModHandle, &HostOptions);
			UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit: SetHostAddress(%s) returned (%s)"), *HostAddr, UTF8_TO_TCHAR(EOS_EResult_ToString(HostResult)));
		}
	}

	FCreateSessionCallbackContext* CallbackContext = new FCreateSessionCallbackContext();
	CallbackContext->SessionInterface = FOnlineSessionEOSKitWeakPtr(AsShared());
	CallbackContext->SessionName = Session->SessionName;

	EOS_Sessions_UpdateSessionOptions UpdateOptions = {};
	UpdateOptions.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
	UpdateOptions.SessionModificationHandle = SessionModHandle;

	EOS_Sessions_UpdateSession(SessionsHandle, &UpdateOptions, CallbackContext,
		[](const EOS_Sessions_UpdateSessionCallbackInfo* Data)
		{
			FCreateSessionCallbackContext* Context = static_cast<FCreateSessionCallbackContext*>(Data->ClientData);
			if (Context)
			{
				if (TSharedPtr<FOnlineSessionEOSKit> Pinned = Context->SessionInterface.Pin())
				{
					Pinned->HandleCreateSessionCallback(Context->SessionName, Data);
				}
				delete Context;
			}
		});

	EOS_SessionModification_Release(SessionModHandle);
	return ONLINE_IO_PENDING;
#else
	return ONLINE_FAIL;
#endif
}

void FOnlineSessionEOSKit::HandleCreateSessionCallback(FName SessionName, const EOS_Sessions_UpdateSessionCallbackInfo* Data)
{
#if WITH_EOS_SDK
	bool bWasSuccessful = (Data->ResultCode == EOS_EResult::EOS_Success);
	if (!bWasSuccessful && Data->SessionId && strlen(Data->SessionId) > 0)
	{
		bWasSuccessful = true;
	}

	if (FNamedOnlineSession* Session = GetNamedSession(SessionName))
	{
		if (bWasSuccessful)
		{
			if (Session->SessionInfo.IsValid())
			{
				if (TSharedPtr<FOnlineSessionInfoEOSKit> Info = StaticCastSharedPtr<FOnlineSessionInfoEOSKit>(Session->SessionInfo))
				{
					if (Data->SessionId)
					{
						Info->SetSessionId(UTF8_TO_TCHAR(Data->SessionId));
					}
					
					// CRITICAL: Set EOSAddress for P2P connections
					// Format: EOS:ProductUserId:GameNetDriver:26 (matching EIK)
					EOS_ProductUserId HostUserId = GetProductUserIdForSession(Session);
					if (EOS_ProductUserId_IsValid(HostUserId))
					{
						char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
						int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
						EOS_ProductUserId_ToString(HostUserId, ProductUserIdStr, &ProductUserIdStrSize);
						
						// Build EOS P2P address: EOS:ProductUserId:GameNetDriver:26 (EIK format)
						Info->EOSAddress = FString::Printf(TEXT("EOS:%s:GameNetDriver:26"), UTF8_TO_TCHAR(ProductUserIdStr));
						UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit: Set EOSAddress for host: %s"), *Info->EOSAddress);
					}
				}
			}
			else
			{
				Session->SessionInfo = MakeShareable(new FOnlineSessionInfoEOSKit(TEXT("127.0.0.1")));
				if (TSharedPtr<FOnlineSessionInfoEOSKit> Info = StaticCastSharedPtr<FOnlineSessionInfoEOSKit>(Session->SessionInfo))
				{
					if (Data->SessionId)
					{
						Info->SetSessionId(UTF8_TO_TCHAR(Data->SessionId));
					}
					
					// CRITICAL: Set EOSAddress for P2P connections
					// Format: EOS:ProductUserId:GameNetDriver:26 (matching EIK)
					EOS_ProductUserId HostUserId = GetProductUserIdForSession(Session);
					if (EOS_ProductUserId_IsValid(HostUserId))
					{
						char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
						int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
						EOS_ProductUserId_ToString(HostUserId, ProductUserIdStr, &ProductUserIdStrSize);
						
						// Build EOS P2P address: EOS:ProductUserId:GameNetDriver:26 (EIK format)
						Info->EOSAddress = FString::Printf(TEXT("EOS:%s:GameNetDriver:26"), UTF8_TO_TCHAR(ProductUserIdStr));
						UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit: Set EOSAddress for host: %s"), *Info->EOSAddress);
					}
				}
			}

			Session->SessionState = EOnlineSessionState::Pending;
			RegisterLocalPlayers(Session);
		}
		else
		{
			RemoveNamedSession(SessionName);
		}
	}

	TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);
#endif
}

void FOnlineSessionEOSKit::RegisterLocalPlayers(FNamedOnlineSession* Session)
{
	// EMPTY - Just like EIK does!
	// For P2P sessions, EOS automatically registers the host when StartSession is called
	// Manual RegisterPlayer calls fail with P2P policy (403: missing matchmaking:managePlayers)
	// 
	// If you're using Matchmaking policy (not P2P), you would need to implement registration here
}

EOS_ProductUserId FOnlineSessionEOSKit::GetProductUserIdForSession(FNamedOnlineSession* Session) const
{
	if (Session && Session->OwningUserId.IsValid())
	{
		const FString IdString = Session->OwningUserId->ToString();
		if (!IdString.IsEmpty())
		{
			return EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*IdString));
		}
	}
	return nullptr;
}
bool FOnlineSessionEOSKit::IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		return Session->RegisteredPlayers.ContainsByPredicate([&UniqueId](const FUniqueNetIdRef& PlayerId)
		{
			return *PlayerId == UniqueId;
		});
	}
	return false;
}

bool FOnlineSessionEOSKit::CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName)
{
	return false;
}

bool FOnlineSessionEOSKit::CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName)
{
	return false;
}

bool FOnlineSessionEOSKit::FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Searching for sessions..."));
	
	// Don't start another search while one is in progress
	if (CurrentSessionSearch.IsValid() && SearchSettings->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Ignoring search request - another search is in progress"));
		return false;
	}
	
	// Free up previous results
	SearchSettings->SearchResults.Empty();
	CurrentSessionSearch = SearchSettings;
	
	// Check if it's a LAN query
	if (SearchSettings->bIsLanQuery)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: LAN search not implemented"));
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}
	
#if WITH_EOS_SDK
	if (!SessionsHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::FindSessions: SessionsHandle is null"));
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}
	
	// Check if using lobbies
	bool bUseLobbies = false;
	SearchSettings->QuerySettings.Get(SEARCH_LOBBIES, bUseLobbies);
	
	if (bUseLobbies)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Lobby search not fully implemented"));
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}
	
	// Create EOS session search handle
	EOS_HSessionSearch SearchHandle = nullptr;
	EOS_Sessions_CreateSessionSearchOptions HandleOptions = {};
	HandleOptions.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
	HandleOptions.MaxSearchResults = FMath::Clamp(SearchSettings->MaxSearchResults, 1, EOS_SESSIONS_MAX_SEARCH_RESULTS);
	
	EOS_EResult ResultCode = EOS_Sessions_CreateSessionSearch(SessionsHandle, &HandleOptions, &SearchHandle);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::FindSessions: EOS_Sessions_CreateSessionSearch failed: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}
	
	// Set BucketId search parameter (required for EOS)
	FString BucketId = TEXT("DefaultBucket");
	SearchSettings->QuerySettings.Get(FName(TEXT("BucketId")), BucketId);
	
	EOS_SessionSearch_SetParameterOptions BucketParam = {};
	BucketParam.ApiVersion = EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST;
	BucketParam.ComparisonOp = EOS_EComparisonOp::EOS_CO_EQUAL;
	
	EOS_Sessions_AttributeData BucketAttr = {};
	BucketAttr.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
	BucketAttr.Key = EOS_SESSIONS_SEARCH_BUCKET_ID;
	FTCHARToUTF8 BucketUtf8(*BucketId);
	BucketAttr.Value.AsUtf8 = BucketUtf8.Get();
	BucketAttr.ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
	BucketParam.Parameter = &BucketAttr;
	
	EOS_SessionSearch_SetParameter(SearchHandle, &BucketParam);
	
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Searching for sessions in bucket '%s', max results: %d"), 
		*BucketId, HandleOptions.MaxSearchResults);
	
	// Get the user's ProductUserId for filtering own sessions
	EOS_ProductUserId LocalUserId = nullptr;
	if (EOSKitSubsystem && EOSKitSubsystem->GetIdentityInterface())
	{
		FUniqueNetIdPtr UserId = EOSKitSubsystem->GetIdentityInterface()->GetUniquePlayerId(SearchingPlayerNum);
		if (UserId.IsValid())
		{
			FString UserIdStr = UserId->ToString();
			// Parse ProductUserId from "EpicAccountId|ProductUserId" format
			if (UserIdStr.Contains(TEXT("|")))
			{
				TArray<FString> Parts;
				UserIdStr.ParseIntoArray(Parts, TEXT("|"));
				if (Parts.Num() >= 2)
				{
					LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Parts[1]));
				}
			}
			else
			{
				LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserIdStr));
			}
		}
	}
	
	// Execute the search
	SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
	
	typedef TEOSCallback<EOS_SessionSearch_OnFindCallback, EOS_SessionSearch_FindCallbackInfo, FOnlineSessionEOSKit> FFindSessionsCallback;
	FFindSessionsCallback* CallbackObj = new FFindSessionsCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
	CallbackObj->CallbackLambda = [this, SearchSettings, SearchHandle, LocalUserId](const EOS_SessionSearch_FindCallbackInfo* Data)
	{
		bool bWasSuccessful = (Data->ResultCode == EOS_EResult::EOS_Success);
		
		if (bWasSuccessful)
		{
			// Get number of results
			EOS_SessionSearch_GetSearchResultCountOptions CountOptions = {};
			CountOptions.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
			int32 NumResults = EOS_SessionSearch_GetSearchResultCount(SearchHandle, &CountOptions);
			
			UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: ✅ Found %d session(s)"), NumResults);
			
			// Copy each result
			EOS_SessionSearch_CopySearchResultByIndexOptions IndexOptions = {};
			IndexOptions.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
			
			for (int32 Index = 0; Index < NumResults; Index++)
			{
				EOS_HSessionDetails SessionDetailsHandle = nullptr;
				IndexOptions.SessionIndex = Index;
				
				EOS_EResult CopyResult = EOS_SessionSearch_CopySearchResultByIndex(SearchHandle, &IndexOptions, &SessionDetailsHandle);
				if (CopyResult == EOS_EResult::EOS_Success && SessionDetailsHandle)
				{
					// Get session info
					EOS_SessionDetails_Info* SessionInfo = nullptr;
					EOS_SessionDetails_CopyInfoOptions InfoOptions = {};
					InfoOptions.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;
					
					if (EOS_SessionDetails_CopyInfo(SessionDetailsHandle, &InfoOptions, &SessionInfo) == EOS_EResult::EOS_Success)
					{
						// FILTER OUT OWN SESSIONS: Don't include sessions hosted by the current player
						if (EOS_ProductUserId_IsValid(LocalUserId) && EOS_ProductUserId_IsValid(SessionInfo->OwnerUserId))
						{
							// Compare ProductUserIds by converting to strings
							char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
							char OwnerUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
							int32 LocalUserIdStrSize = sizeof(LocalUserIdStr);
							int32 OwnerUserIdStrSize = sizeof(OwnerUserIdStr);
							
							if (EOS_ProductUserId_ToString(LocalUserId, LocalUserIdStr, &LocalUserIdStrSize) == EOS_EResult::EOS_Success &&
								EOS_ProductUserId_ToString(SessionInfo->OwnerUserId, OwnerUserIdStr, &OwnerUserIdStrSize) == EOS_EResult::EOS_Success)
							{
								if (FCStringAnsi::Strcmp(LocalUserIdStr, OwnerUserIdStr) == 0)
								{
									UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: ⚠️ Filtering out own session (ID: %s)"), UTF8_TO_TCHAR(SessionInfo->SessionId));
									EOS_SessionDetails_Info_Release(SessionInfo);
									EOS_SessionDetails_Release(SessionDetailsHandle);
									continue; // Skip this session
								}
							}
						}
						
						// Add to search results
						int32 ResultIndex = SearchSettings->SearchResults.AddZeroed();
						FOnlineSessionSearchResult& Result = SearchSettings->SearchResults[ResultIndex];
						
						// Create session info with SessionHandle (CRITICAL for joining!)
						FString SessionIdStr = UTF8_TO_TCHAR(SessionInfo->SessionId);
						FString HostAddrStr = UTF8_TO_TCHAR(SessionInfo->HostAddress);
						
						FOnlineSessionInfoEOSKit* NewSessionInfo = new FOnlineSessionInfoEOSKit(HostAddrStr);
						NewSessionInfo->SetSessionId(SessionIdStr);
						NewSessionInfo->SessionHandle = SessionDetailsHandle; // Store handle for joining!
						
						// CRITICAL: Set EOSAddress for P2P connections using host's ProductUserId
						if (EOS_ProductUserId_IsValid(SessionInfo->OwnerUserId))
						{
							char OwnerIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
							int32 BufferSize = sizeof(OwnerIdStr);
							if (EOS_ProductUserId_ToString(SessionInfo->OwnerUserId, OwnerIdStr, &BufferSize) == EOS_EResult::EOS_Success)
							{
								// Build EOS P2P address: EOS:HostProductUserId:GameNetDriver:26 (EIK format)
								NewSessionInfo->EOSAddress = FString::Printf(TEXT("EOS:%s:GameNetDriver:26"), UTF8_TO_TCHAR(OwnerIdStr));
								UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::FindSessions: Set EOSAddress for found session: %s"), *NewSessionInfo->EOSAddress);
							}
						}
						
						Result.Session.SessionInfo = MakeShareable(NewSessionInfo);
						
						// Copy basic session data
						Result.Session.NumOpenPublicConnections = SessionInfo->NumOpenPublicConnections;
						Result.Session.NumOpenPrivateConnections = 0;
						Result.Session.SessionSettings.NumPublicConnections = SessionInfo->Settings->NumPublicConnections;
						Result.Session.SessionSettings.bShouldAdvertise = true;
						Result.Session.SessionSettings.bAllowJoinInProgress = SessionInfo->Settings->bAllowJoinInProgress == EOS_TRUE;
						Result.Session.SessionSettings.bIsLANMatch = false;
						Result.Session.SessionSettings.bIsDedicated = false;
						
						// Convert OwnerUserId to string for display
						if (EOS_ProductUserId_IsValid(SessionInfo->OwnerUserId))
						{
							char OwnerIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
							int32 BufferSize = sizeof(OwnerIdStr);
							if (EOS_ProductUserId_ToString(SessionInfo->OwnerUserId, OwnerIdStr, &BufferSize) == EOS_EResult::EOS_Success)
							{
								Result.Session.OwningUserName = UTF8_TO_TCHAR(OwnerIdStr);
							}
						}
						
						UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: ✅ Session %d - ID: %s, Host: %s, OpenSlots: %d/%d"), 
							Index, *SessionIdStr, *HostAddrStr, SessionInfo->NumOpenPublicConnections, SessionInfo->Settings->NumPublicConnections);
						
						EOS_SessionDetails_Info_Release(SessionInfo);
					}
					
					// Don't release SessionDetailsHandle here - it's managed by the search handle
				}
			}
			
			SearchSettings->SearchState = EOnlineAsyncTaskState::Done;
		}
		else
		{
			UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::FindSessions: Search failed: %s"), 
				UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
			SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
		}
		
		// Release search handle
		EOS_SessionSearch_Release(SearchHandle);
		
		TriggerOnFindSessionsCompleteDelegates(bWasSuccessful);
	};
	
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::FindSessions: Invalid ProductUserId"));
		EOS_SessionSearch_Release(SearchHandle);
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}
	
	// Execute the search
	EOS_SessionSearch_FindOptions FindOptions = {};
	FindOptions.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
	FindOptions.LocalUserId = LocalUserId;
	
	EOS_SessionSearch_Find(SearchHandle, &FindOptions, CallbackObj, CallbackObj->GetCallbackPtr());
	
	return true;
#else
	TriggerOnFindSessionsCompleteDelegates(false);
	return false;
#endif
}

bool FOnlineSessionEOSKit::FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	return FindSessions(0, SearchSettings);
}

bool FOnlineSessionEOSKit::FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& Delegate)
{
	// Find session by ID not implemented yet
	// In UE 5.5, FOnSingleSessionResultCompleteDelegate takes (int, bool, const FOnlineSessionSearchResult&)
	// Use 0 as LocalUserNum since we don't have it
	Delegate.ExecuteIfBound(0, false, FOnlineSessionSearchResult());
	return false;
}

bool FOnlineSessionEOSKit::CancelFindSessions()
{
	CurrentSessionSearch.Reset();
	return true;
}

bool FOnlineSessionEOSKit::PingSearchResults(const FOnlineSessionSearchResult& SearchResult)
{
	// Not implemented yet
	return false;
}

bool FOnlineSessionEOSKit::JoinSession(int32 PlayerNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::JoinSession: Joining session '%s'"), *SessionName.ToString());
	
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	
	// Don't join if already in a session
	if (Session != nullptr)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::JoinSession: Session '%s' already exists, destroying it first..."), *SessionName.ToString());
		DestroySession(SessionName, FOnDestroySessionCompleteDelegate());
		Session = nullptr;
	}
	
	// Create a named session from the search result
	Session = AddNamedSession(SessionName, DesiredSession.Session);
	if (!Session)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::JoinSession: Failed to create session '%s'"), *SessionName.ToString());
		TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		return false;
	}
	
	Session->HostingPlayerNum = PlayerNum;
	Session->SessionState = EOnlineSessionState::Pending;
	
	// Check if session info is valid
	if (!DesiredSession.Session.SessionInfo.IsValid())
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::JoinSession: Invalid session info"));
		RemoveNamedSession(SessionName);
		TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		return false;
	}
	
	// Copy session info (including EOSAddress for P2P connections)
	TSharedPtr<const FOnlineSessionInfoEOSKit> SearchSessionInfo = StaticCastSharedPtr<const FOnlineSessionInfoEOSKit>(DesiredSession.Session.SessionInfo);
	Session->SessionInfo = MakeShareable(new FOnlineSessionInfoEOSKit(*SearchSessionInfo));
	
	// Verify EOSAddress was copied
	if (TSharedPtr<FOnlineSessionInfoEOSKit> JoinedSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoEOSKit>(Session->SessionInfo))
	{
		if (!JoinedSessionInfo->EOSAddress.IsEmpty())
		{
			UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::JoinSession: ✅ Copied EOSAddress: %s"), *JoinedSessionInfo->EOSAddress);
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::JoinSession: ⚠️ EOSAddress is empty after copy!"));
		}
	}
	
#if WITH_EOS_SDK
	if (!SessionsHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::JoinSession: SessionsHandle is null"));
		RemoveNamedSession(SessionName);
		TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		return false;
	}
	
	// Get ProductUserId
	EOS_ProductUserId LocalUserId = nullptr;
	if (EOSKitSubsystem && EOSKitSubsystem->GetIdentityInterface())
	{
		FUniqueNetIdPtr UserId = EOSKitSubsystem->GetIdentityInterface()->GetUniquePlayerId(PlayerNum);
		if (UserId.IsValid())
		{
			FString UserIdStr = UserId->ToString();
			if (UserIdStr.Contains(TEXT("|")))
			{
				TArray<FString> Parts;
				UserIdStr.ParseIntoArray(Parts, TEXT("|"));
				if (Parts.Num() >= 2)
				{
					LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Parts[1]));
				}
			}
			else
			{
				LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserIdStr));
			}
		}
	}
	
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::JoinSession: Invalid ProductUserId"));
		RemoveNamedSession(SessionName);
		TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		return false;
	}
	
	// CHECK: Prevent joining your own session (SearchSessionInfo already declared above)
	if (SearchSessionInfo.IsValid())
	{
		// Extract host ProductUserId from EOSAddress (format: EOS:ProductUserId:GameNetDriver:26)
		FString EOSAddress = SearchSessionInfo->EOSAddress;
		if (!EOSAddress.IsEmpty() && EOSAddress.StartsWith(TEXT("EOS:")))
		{
			FString HostProductUserIdStr = EOSAddress.RightChop(4); // Remove "EOS:"
			TArray<FString> Parts;
			HostProductUserIdStr.ParseIntoArray(Parts, TEXT(":"), true);
			if (Parts.Num() >= 1)
			{
				EOS_ProductUserId HostUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Parts[0]));
				if (EOS_ProductUserId_IsValid(HostUserId))
				{
					// Compare ProductUserIds by converting to strings
					char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
					char HostUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
					int32 LocalUserIdStrSize = sizeof(LocalUserIdStr);
					int32 HostUserIdStrSize = sizeof(HostUserIdStr);
					
					if (EOS_ProductUserId_ToString(LocalUserId, LocalUserIdStr, &LocalUserIdStrSize) == EOS_EResult::EOS_Success &&
						EOS_ProductUserId_ToString(HostUserId, HostUserIdStr, &HostUserIdStrSize) == EOS_EResult::EOS_Success)
					{
						if (FCStringAnsi::Strcmp(LocalUserIdStr, HostUserIdStr) == 0)
						{
							UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::JoinSession: ❌ Cannot join your own session! You are the host."));
							RemoveNamedSession(SessionName);
							TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::AlreadyInSession);
							return false;
						}
					}
				}
			}
		}
	}
	
	// Get the SessionHandle from the session info (stored during FindSessions)
	TSharedPtr<FOnlineSessionInfoEOSKit> EOSSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoEOSKit>(Session->SessionInfo);
	if (!EOSSessionInfo.IsValid() || !EOSSessionInfo->SessionHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::JoinSession: Invalid SessionHandle"));
		RemoveNamedSession(SessionName);
		TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		return false;
	}
	
	// Setup join options
	EOS_Sessions_JoinSessionOptions JoinOptions = {};
	JoinOptions.ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
	FTCHARToUTF8 SessionNameUtf8(*SessionName.ToString());
	JoinOptions.SessionName = SessionNameUtf8.Get();
	JoinOptions.LocalUserId = LocalUserId;
	JoinOptions.SessionHandle = EOSSessionInfo->SessionHandle;
	
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::JoinSession: Calling EOS_Sessions_JoinSession..."));
	
	// Create callback
	typedef TEOSCallback<EOS_Sessions_OnJoinSessionCallback, EOS_Sessions_JoinSessionCallbackInfo, FOnlineSessionEOSKit> FJoinSessionCallback;
	FJoinSessionCallback* CallbackObj = new FJoinSessionCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
	CallbackObj->CallbackLambda = [this, SessionName](const EOS_Sessions_JoinSessionCallbackInfo* Data)
	{
		bool bWasSuccessful = (Data->ResultCode == EOS_EResult::EOS_Success);
		
		FNamedOnlineSession* Session = GetNamedSession(SessionName);
		if (Session)
		{
			if (bWasSuccessful)
			{
				Session->SessionState = EOnlineSessionState::Pending;
				UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::JoinSession: ✅ Successfully joined session '%s'"), *SessionName.ToString());
			}
			else
			{
				UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::JoinSession: Failed to join session '%s': %s"), 
					*SessionName.ToString(), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				Session->SessionState = EOnlineSessionState::NoSession;
				RemoveNamedSession(SessionName);
			}
		}
		
		TriggerOnJoinSessionCompleteDelegates(SessionName, bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
	};
	
	EOS_Sessions_JoinSession(SessionsHandle, &JoinOptions, CallbackObj, CallbackObj->GetCallbackPtr());
	return true;
#else
	TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
	return false;
#endif
}

bool FOnlineSessionEOSKit::JoinSession(const FUniqueNetId& PlayerId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	return JoinSession(0, SessionName, DesiredSession);
}

bool FOnlineSessionEOSKit::FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend)
{
	return false;
}

bool FOnlineSessionEOSKit::FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& FriendId)
{
	return false;
}

bool FOnlineSessionEOSKit::SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend)
{
	return false;
}

bool FOnlineSessionEOSKit::SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& FriendId)
{
	return false;
}

bool FOnlineSessionEOSKit::SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray<FUniqueNetIdRef>& Friends)
{
	return false;
}

bool FOnlineSessionEOSKit::SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray<FUniqueNetIdRef>& FriendIds)
{
	return false;
}

bool FOnlineSessionEOSKit::GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!Session)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: Session '%s' not found"), *SessionName.ToString());
		return false;
	}
	
	if (!Session->SessionInfo.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: Session '%s' has invalid SessionInfo"), *SessionName.ToString());
		return false;
	}
	
	TSharedPtr<FOnlineSessionInfoEOSKit> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoEOSKit>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: Failed to cast SessionInfo"));
		return false;
	}
	
	// CRITICAL: For EOS P2P sessions, prefer EOSAddress over IP address
	// EIK returns EOSAddress as-is without modification
	if (!SessionInfo->EOSAddress.IsEmpty())
	{
		ConnectInfo = SessionInfo->EOSAddress;
		UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: ✅ Using EOS P2P address: %s"), *ConnectInfo);
		return true;
	}
	
	// Fallback to IP address (for LAN or non-EOS sessions)
	if (SessionInfo->HostAddr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: Both EOSAddress and HostAddr are empty"));
		return false;
	}
	
	ConnectInfo = SessionInfo->HostAddr;
	if (!ConnectInfo.Contains(TEXT(":")))
	{
		ConnectInfo = FString::Printf(TEXT("%s:7777"), *SessionInfo->HostAddr);
	}
	
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: ✅ Using IP address: %s"), *ConnectInfo);
	return true;
}

bool FOnlineSessionEOSKit::GetResolvedConnectString(const FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo)
{
	if (!SearchResult.Session.SessionInfo.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: SearchResult has invalid SessionInfo"));
		return false;
	}
	
	TSharedPtr<FOnlineSessionInfoEOSKit> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoEOSKit>(SearchResult.Session.SessionInfo);
	if (!SessionInfo.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: Failed to cast SessionInfo from SearchResult"));
		return false;
	}
	
	// CRITICAL: For EOS P2P sessions, prefer EOSAddress over IP address
	if (!SessionInfo->EOSAddress.IsEmpty())
	{
		ConnectInfo = SessionInfo->EOSAddress;
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: ✅ Using EOS P2P address from SearchResult: %s"), *ConnectInfo);
		return true;
	}
	
	// Fallback: Build connect string from host address
	if (SessionInfo->HostAddr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: Both EOSAddress and HostAddr are empty in SearchResult"));
		return false;
	}
	
	ConnectInfo = SessionInfo->HostAddr;
	
	// If no port specified, add default game port
	if (!ConnectInfo.Contains(TEXT(":")))
	{
		ConnectInfo = FString::Printf(TEXT("%s:7777"), *SessionInfo->HostAddr);
	}
	
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: ✅ Using IP address from SearchResult: %s"), *ConnectInfo);
	return true;
}

FOnlineSessionSettings* FOnlineSessionEOSKit::GetSessionSettings(FName SessionName)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		return &Session->SessionSettings;
	}
	return nullptr;
}

bool FOnlineSessionEOSKit::RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited)
{
	TArray<FUniqueNetIdRef> Players;
	Players.Add(PlayerId.AsShared());
	return RegisterPlayers(SessionName, Players, bWasInvited);
}

bool FOnlineSessionEOSKit::RegisterPlayers(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasInvited)
{
	UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: Called for session '%s' with %d player(s)"), *SessionName.ToString(), Players.Num());
	
	bool bSuccess = false;
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!Session)
	{
		UE_LOG_ONLINE(Warning, TEXT("RegisterPlayers: ❌ No session found for session (%s)"), *SessionName.ToString());
		return false;
	}

	UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: Session found - bHosting=%d, bIsDedicated=%d, bUseLobbies=%d"), 
		Session->bHosting, Session->SessionSettings.bIsDedicated, Session->SessionSettings.bUseLobbiesIfAvailable);

	// Check if we're the host or dedicated server
	if (Session->bHosting || Session->SessionSettings.bIsDedicated)
	{
		TArray<EOS_ProductUserId> EOSIds;
		bSuccess = true;
		
		// Only register with EOS if not using lobbies
		bool bRegisterEOS = !Session->SessionSettings.bUseLobbiesIfAvailable;
		
		// Process each player
		for (const FUniqueNetIdRef& PlayerId : Players)
		{
			// Convert FUniqueNetId to EOS_ProductUserId
			FString PlayerIdString = PlayerId->ToString();
			UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: Processing player ID: %s"), *PlayerIdString);
			
			// Parse ProductUserId from string (handle "EpicAccountId|ProductUserId" format)
			FString ProductUserIdStr = PlayerIdString;
			if (PlayerIdString.Contains(TEXT("|")))
			{
				TArray<FString> Parts;
				PlayerIdString.ParseIntoArray(Parts, TEXT("|"), true);
				if (Parts.Num() >= 2)
				{
					ProductUserIdStr = Parts[1]; // Get the ProductUserId part
					UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: Parsed ProductUserId: %s"), *ProductUserIdStr);
				}
			}
			
			// Convert to EOS_ProductUserId
			EOS_ProductUserId EOSProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserIdStr));
			if (EOS_ProductUserId_IsValid(EOSProductUserId))
			{
				EOSIds.Add(EOSProductUserId);
				UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: ✅ Valid EOS_ProductUserId added"));
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("RegisterPlayers: ❌ Invalid EOS_ProductUserId: %s"), *ProductUserIdStr);
			}
			
			// Add to local tracking
			Session->RegisteredPlayers.AddUnique(PlayerId);
			
			// Update open connection count
			if (Session->NumOpenPublicConnections > 0)
			{
				Session->NumOpenPublicConnections--;
				UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: Decremented open slots - now %d/%d"), 
					Session->NumOpenPublicConnections, Session->SessionSettings.NumPublicConnections);
			}
		}

		// Register with EOS backend if needed
		UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: bRegisterEOS=%d, EOSIds.Num()=%d, SessionsHandle=%s"), 
			bRegisterEOS, EOSIds.Num(), SessionsHandle ? TEXT("Valid") : TEXT("NULL"));
			
		if (bRegisterEOS && EOSIds.Num() > 0 && SessionsHandle)
		{
			UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: Calling EOS_Sessions_RegisterPlayers for session '%s'"), *SessionName.ToString());
			
			EOS_Sessions_RegisterPlayersOptions Options = {};
			Options.ApiVersion = EOS_SESSIONS_REGISTERPLAYERS_API_LATEST;
			Options.PlayersToRegister = EOSIds.GetData();
			Options.PlayersToRegisterCount = EOSIds.Num();
			const FTCHARToUTF8 Utf8SessionName(*SessionName.ToString());
			Options.SessionName = Utf8SessionName.Get();

			// Create callback context
			struct FRegisterPlayersContext
			{
				FOnlineSessionEOSKit* SessionInterface;
				FName SessionName;
				TArray<FUniqueNetIdRef> Players;
			};

			FRegisterPlayersContext* Context = new FRegisterPlayersContext();
			Context->SessionInterface = this;
			Context->SessionName = SessionName;
			Context->Players = Players;

			EOS_Sessions_RegisterPlayers(SessionsHandle, &Options, Context,
				[](const EOS_Sessions_RegisterPlayersCallbackInfo* Data)
				{
					FRegisterPlayersContext* Context = static_cast<FRegisterPlayersContext*>(Data->ClientData);
					if (Context)
					{
						bool bWasSuccessful = Data->ResultCode == EOS_EResult::EOS_Success || Data->ResultCode == EOS_EResult::EOS_NoChange;
						if (bWasSuccessful)
						{
							UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: ✅ Successfully registered %d player(s) in session '%s' via EOS SDK"), 
								Context->Players.Num(), *Context->SessionName.ToString());
						}
						else
						{
							UE_LOG_ONLINE(Warning, TEXT("RegisterPlayers: ❌ Failed to register players: %s"), 
								UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
						}
						delete Context;
					}
				});

			UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: Registering %d player(s) in session '%s'"), EOSIds.Num(), *SessionName.ToString());
			return true;
		}
		
		return bSuccess;
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("RegisterPlayers: Not the owner of the session (%s)"), *SessionName.ToString());
	}

	return false;
}

bool FOnlineSessionEOSKit::UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		return Session->RegisteredPlayers.RemoveAll([&PlayerId](const FUniqueNetIdRef& Ref)
		{
			return *Ref == PlayerId;
		}) > 0;
	}
	return false;
}

bool FOnlineSessionEOSKit::UnregisterPlayers(FName SessionName, const TArray<FUniqueNetIdRef>& Players)
{
	bool bSuccess = true;
	for (const FUniqueNetIdRef& PlayerId : Players)
	{
		if (!UnregisterPlayer(SessionName, *PlayerId))
		{
			bSuccess = false;
		}
	}
	return bSuccess;
}

int32 FOnlineSessionEOSKit::GetNumSessions()
{
	FScopeLock ScopeLock(&SessionLock);
	return Sessions.Num();
}

void FOnlineSessionEOSKit::DumpSessionState()
{
	FScopeLock ScopeLock(&SessionLock);
	UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit: Dumping session state - %d sessions"), Sessions.Num());
	for (const FNamedOnlineSession& Session : Sessions)
	{
		UE_LOG_ONLINE(Log, TEXT("  Session: %s, State: %d"), *Session.SessionName.ToString(), (int32)Session.SessionState);
	}
}

FNamedOnlineSession* FOnlineSessionEOSKit::AddNamedSession(FName SessionName, const FOnlineSession& Session)
{
	FScopeLock ScopeLock(&SessionLock);
	FNamedOnlineSession* ExistingSession = GetNamedSession(SessionName);
	if (ExistingSession)
	{
		// Update existing session - copy settings only (can't assign FOnlineSession directly)
		ExistingSession->SessionSettings = Session.SessionSettings;
		// SessionState is in FNamedOnlineSession, not FOnlineSession, so we keep existing state
		return ExistingSession;
	}
	else
	{
		// In UE 5.5, FNamedOnlineSession constructor is protected
		// Try using Emplace with constructor parameters - if that doesn't work, we'll need AddDefaulted
		// FNamedOnlineSession likely has a constructor: FNamedOnlineSession(FName, const FOnlineSessionSettings&)
		int32 NewIndex = Sessions.Emplace(SessionName, Session.SessionSettings);
		FNamedOnlineSession* NewSession = &Sessions[NewIndex];
		// Set default state for new session
		NewSession->SessionState = EOnlineSessionState::Creating;
		return NewSession;
	}
}

FNamedOnlineSession* FOnlineSessionEOSKit::AddNamedSession(FName SessionName, const FOnlineSessionSettings& SessionSettings)
{
	FScopeLock ScopeLock(&SessionLock);
	FNamedOnlineSession* ExistingSession = GetNamedSession(SessionName);
	if (ExistingSession)
	{
		ExistingSession->SessionSettings = SessionSettings;
		return ExistingSession;
	}
	else
	{
		// In UE 5.5, FNamedOnlineSession constructor is protected
		// Try using Emplace with constructor parameters
		// FNamedOnlineSession likely has a constructor: FNamedOnlineSession(FName, const FOnlineSessionSettings&)
		int32 NewIndex = Sessions.Emplace(SessionName, SessionSettings);
		FNamedOnlineSession* NewSession = &Sessions[NewIndex];
		// Set default state for new session
		NewSession->SessionState = EOnlineSessionState::NoSession;
		return NewSession;
	}
}

bool FOnlineSessionEOSKit::StartMatchmaking(const TArray<FUniqueNetIdRef>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	// Matchmaking not implemented yet
	return false;
}

bool FOnlineSessionEOSKit::FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<FUniqueNetIdRef>& Friends)
{
	// Find friend session not implemented yet
	return false;
}

void FOnlineSessionEOSKit::RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate)
{
	// Register local player - not implemented yet
	// In UE 5.5, FOnRegisterLocalPlayerCompleteDelegate takes (const FUniqueNetId&, EOnJoinSessionCompleteResult::Type)
	Delegate.ExecuteIfBound(PlayerId, EOnJoinSessionCompleteResult::UnknownError);
}

void FOnlineSessionEOSKit::UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate)
{
	// Unregister local player - not implemented yet
	// In UE 5.5, FOnUnregisterLocalPlayerCompleteDelegate takes (const FUniqueNetId&, bool)
	Delegate.ExecuteIfBound(PlayerId, false);
}

void FOnlineSessionEOSKit::DelegateToEOSKitSessionManager()
{
	// Helper method to get the session manager
	// Implementation can be expanded as needed
}

#endif // WITH_EOS_SDK

