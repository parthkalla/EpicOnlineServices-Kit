// Copyright (C) 2024, All Rights Reserved.

#include "OnlineSessionEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitSessionManager.h"
#include "EOSKitSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineBase.h"
#include "OnlineSubsystemTypes.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/OnlineSessionNames.h"
#endif
#include "HAL/PlatformProcess.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

#if WITH_EOS_SDK

#include "eos_lobby.h"
#include "eos_lobby_types.h"
#include "eos_sessions_types.h"
#include "Misc/Guid.h"
#include "Kismet/GameplayStatics.h"
#include "Async/Async.h"
#include "InternetAddrEOS.h"
#include "NetDriverEOS.h"
#include "SocketSubsystem.h"

// Define FUniqueNetIdEOSLobby (like EIK does - but EIK actually defines it in header, we'll keep it here for now)
static FName EOS_LOBBY_ID = TEXT("EOSLobby");
TEMP_UNIQUENETIDSTRING_SUBCLASS(FUniqueNetIdEOSLobby, EOS_LOBBY_ID);
// Ensure ref type matches header alias
// The macro typically defines FUniqueNetIdEOSLobbyRef as TSharedRef<const FUniqueNetIdEOSLobby>
// so header now uses const as well.

/** This is the game name plus version in ansi done once for optimization */
char BucketIdAnsi[EOS_OSS_STRING_BUFFER_LENGTH];

// SEARCH_LOBBIES constant (like EIK uses)
#ifndef SEARCH_LOBBIES
static const FName SEARCH_LOBBIES(TEXT("SEARCH_LOBBIES"));
#endif

FString MakeStringFromAttributeValue(const EOS_Sessions_AttributeData* Attribute)
{
	switch (Attribute->ValueType)
	{
		case EOS_ESessionAttributeType::EOS_SAT_Int64:
		{
			int32 Value = Attribute->Value.AsInt64;
			return FString::Printf(TEXT("%d"), Value);
		}
		case EOS_ESessionAttributeType::EOS_SAT_Double:
		{
			double Value = Attribute->Value.AsDouble;
			return FString::Printf(TEXT("%f"), Value);
		}
		case EOS_ESessionAttributeType::EOS_SAT_String:
		{
			return FString(UTF8_TO_TCHAR(Attribute->Value.AsUtf8));
		}
	}
	return TEXT("");
}

// Forward declare for TEOSCallback
class FOnlineSessionEOSKit;
typedef TWeakPtr<FOnlineSessionEOSKit, ESPMode::ThreadSafe> FOnlineSessionEOSKitWeakPtr;

// Lobby callback types
#if ENGINE_MAJOR_VERSION == 5
typedef TEOSCallback<EOS_Lobby_OnCreateLobbyCallback, EOS_Lobby_CreateLobbyCallbackInfo, FOnlineSessionEOSKit> FLobbyCreatedCallback;
typedef TEOSCallback<EOS_Lobby_OnUpdateLobbyCallback, EOS_Lobby_UpdateLobbyCallbackInfo, FOnlineSessionEOSKit> FLobbyUpdatedCallback;
typedef TEOSCallback<EOS_Lobby_OnJoinLobbyCallback, EOS_Lobby_JoinLobbyCallbackInfo, FOnlineSessionEOSKit> FLobbyJoinedCallback;
typedef TEOSCallback<EOS_Lobby_OnLeaveLobbyCallback, EOS_Lobby_LeaveLobbyCallbackInfo, FOnlineSessionEOSKit> FLobbyLeftCallback;
typedef TEOSCallback<EOS_Lobby_OnDestroyLobbyCallback, EOS_Lobby_DestroyLobbyCallbackInfo, FOnlineSessionEOSKit> FLobbyDestroyedCallback;
typedef TEOSCallback<EOS_Lobby_OnSendInviteCallback, EOS_Lobby_SendInviteCallbackInfo, FOnlineSessionEOSKit> FLobbySendInviteCallback;
typedef TEOSCallback<EOS_LobbySearch_OnFindCallback, EOS_LobbySearch_FindCallbackInfo, FOnlineSessionEOSKit> FLobbySearchFindCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnLobbyUpdateReceivedCallback, EOS_Lobby_LobbyUpdateReceivedCallbackInfo, FOnlineSessionEOSKit> FLobbyUpdateReceivedCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnLobbyMemberUpdateReceivedCallback, EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo, FOnlineSessionEOSKit> FLobbyMemberUpdateReceivedCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnLobbyMemberStatusReceivedCallback, EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo, FOnlineSessionEOSKit> FLobbyMemberStatusReceivedCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnLobbyInviteAcceptedCallback, EOS_Lobby_LobbyInviteAcceptedCallbackInfo, FOnlineSessionEOSKit> FLobbyInviteAcceptedCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnJoinLobbyAcceptedCallback, EOS_Lobby_JoinLobbyAcceptedCallbackInfo, FOnlineSessionEOSKit> FJoinLobbyAcceptedCallback;
#if PLATFORM_WINDOWS
typedef TEIKGlobalCallback<EOS_Lobby_OnLeaveLobbyRequestedCallback, EOS_Lobby_LeaveLobbyRequestedCallbackInfo, FOnlineSessionEOSKit> FLeaveLobbyRequestCallback;
#endif

// Session search callback typedef (from EIK)
typedef TEOSCallback<EOS_SessionSearch_OnFindCallback, EOS_SessionSearch_FindCallbackInfo, FOnlineSessionEOSKit> FFindSessionsCallback;
#else
typedef TEOSCallback<EOS_Lobby_OnCreateLobbyCallback, EOS_Lobby_CreateLobbyCallbackInfo> FLobbyCreatedCallback;
typedef TEOSCallback<EOS_Lobby_OnUpdateLobbyCallback, EOS_Lobby_UpdateLobbyCallbackInfo> FLobbyUpdatedCallback;
typedef TEOSCallback<EOS_Lobby_OnJoinLobbyCallback, EOS_Lobby_JoinLobbyCallbackInfo> FLobbyJoinedCallback;
typedef TEOSCallback<EOS_Lobby_OnLeaveLobbyCallback, EOS_Lobby_LeaveLobbyCallbackInfo> FLobbyLeftCallback;
typedef TEOSCallback<EOS_Lobby_OnDestroyLobbyCallback, EOS_Lobby_DestroyLobbyCallbackInfo> FLobbyDestroyedCallback;
typedef TEOSCallback<EOS_Lobby_OnSendInviteCallback, EOS_Lobby_SendInviteCallbackInfo> FLobbySendInviteCallback;
typedef TEOSCallback<EOS_LobbySearch_OnFindCallback, EOS_LobbySearch_FindCallbackInfo> FLobbySearchFindCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnLobbyUpdateReceivedCallback, EOS_Lobby_LobbyUpdateReceivedCallbackInfo> FLobbyUpdateReceivedCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnLobbyMemberUpdateReceivedCallback, EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo> FLobbyMemberUpdateReceivedCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnLobbyMemberStatusReceivedCallback, EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo> FLobbyMemberStatusReceivedCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnLobbyInviteAcceptedCallback, EOS_Lobby_LobbyInviteAcceptedCallbackInfo> FLobbyInviteAcceptedCallback;
typedef TEIKGlobalCallback<EOS_Lobby_OnJoinLobbyAcceptedCallback, EOS_Lobby_JoinLobbyAcceptedCallbackInfo> FJoinLobbyAcceptedCallback;
#if PLATFORM_WINDOWS
typedef TEIKGlobalCallback<EOS_Lobby_OnLeaveLobbyRequestedCallback, EOS_Lobby_LeaveLobbyRequestedCallbackInfo> FLeaveLobbyRequestCallback;
#endif

// Session search callback typedef (from EIK)
typedef TEOSCallback<EOS_SessionSearch_OnFindCallback, EOS_SessionSearch_FindCallbackInfo> FFindSessionsCallback;
#endif

// Helper structs for attribute data (from EIK)
struct FLobbyAttributeOptions :
	public EOS_Lobby_AttributeData
{
	char KeyAnsi[EOS_OSS_STRING_BUFFER_LENGTH];
	char ValueAnsi[EOS_OSS_STRING_BUFFER_LENGTH];

	FLobbyAttributeOptions(const char* InKey, const char* InValue) :
		EOS_Lobby_AttributeData()
	{
		ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
		ValueType = EOS_ELobbyAttributeType::EOS_SAT_String;
		Value.AsUtf8 = ValueAnsi;
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
		FCStringAnsi::Strncpy(ValueAnsi, InValue, EOS_OSS_STRING_BUFFER_LENGTH);
	}

	FLobbyAttributeOptions(const char* InKey, bool InValue) :
		EOS_Lobby_AttributeData()
	{
		ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
		ValueType = EOS_ELobbyAttributeType::EOS_SAT_Boolean;
		Value.AsBool = InValue ? EOS_TRUE : EOS_FALSE;
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
	}

	FLobbyAttributeOptions(const char* InKey, float InValue) :
		EOS_Lobby_AttributeData()
	{
		ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
		ValueType = EOS_ELobbyAttributeType::EOS_SAT_Double;
		Value.AsDouble = InValue;
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
	}

	FLobbyAttributeOptions(const char* InKey, int32 InValue) :
		EOS_Lobby_AttributeData()
	{
		ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
		ValueType = EOS_ELobbyAttributeType::EOS_SAT_Int64;
		Value.AsInt64 = InValue;
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
	}

	FLobbyAttributeOptions(const char* InKey, const FVariantData& InValue) :
		EOS_Lobby_AttributeData()
	{
		ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;

		switch (InValue.GetType())
		{
		case EOnlineKeyValuePairDataType::Int32:
		{
			ValueType = EOS_ELobbyAttributeType::EOS_SAT_Int64;
			int32 RawValue = 0;
			InValue.GetValue(RawValue);
			Value.AsInt64 = RawValue;
			break;
		}
		case EOnlineKeyValuePairDataType::UInt32:
		{
			ValueType = EOS_ELobbyAttributeType::EOS_SAT_Int64;
			uint32 RawValue = 0;
			InValue.GetValue(RawValue);
			Value.AsInt64 = RawValue;
			break;
		}
		case EOnlineKeyValuePairDataType::Int64:
		{
			ValueType = EOS_ELobbyAttributeType::EOS_SAT_Int64;
			int64 RawValue = 0;
			InValue.GetValue(RawValue);
			Value.AsInt64 = RawValue;
			break;
		}
		case EOnlineKeyValuePairDataType::Bool:
		{
			ValueType = EOS_ELobbyAttributeType::EOS_SAT_Boolean;
			bool RawValue = false;
			InValue.GetValue(RawValue);
			Value.AsBool = RawValue ? EOS_TRUE : EOS_FALSE;
			break;
		}
		case EOnlineKeyValuePairDataType::Double:
		{
			ValueType = EOS_ELobbyAttributeType::EOS_SAT_Double;
			double RawValue = 0.0;
			InValue.GetValue(RawValue);
			Value.AsDouble = RawValue;
			break;
		}
		case EOnlineKeyValuePairDataType::Float:
		{
			ValueType = EOS_ELobbyAttributeType::EOS_SAT_Double;
			float RawValue = 0.0f;
			InValue.GetValue(RawValue);
			Value.AsDouble = RawValue;
			break;
		}
		case EOnlineKeyValuePairDataType::String:
		case EOnlineKeyValuePairDataType::Json:
		{
			ValueType = EOS_ELobbyAttributeType::EOS_SAT_String;
			Value.AsUtf8 = ValueAnsi;
			Key = KeyAnsi;
			FString OutString;
			InValue.GetValue(OutString);
			FCStringAnsi::Strncpy(ValueAnsi, TCHAR_TO_UTF8(*OutString), EOS_OSS_STRING_BUFFER_LENGTH);
			break;
		}
		}
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
	}
};

// FAttributeOptions struct for session attributes (from EIK)
struct FAttributeOptions :
	public EOS_Sessions_AttributeData
{
	char KeyAnsi[EOS_OSS_STRING_BUFFER_LENGTH];
	char ValueAnsi[EOS_OSS_STRING_BUFFER_LENGTH];

	FAttributeOptions(const char* InKey, const char* InValue) :
		EOS_Sessions_AttributeData()
	{
		ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
		Value.AsUtf8 = ValueAnsi;
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
		FCStringAnsi::Strncpy(ValueAnsi, InValue, EOS_OSS_STRING_BUFFER_LENGTH);
	}

	FAttributeOptions(const char* InKey, bool InValue) :
		EOS_Sessions_AttributeData()
	{
		ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		ValueType = EOS_ESessionAttributeType::EOS_SAT_Boolean;
		Value.AsBool = InValue ? EOS_TRUE : EOS_FALSE;
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
	}

	FAttributeOptions(const char* InKey, float InValue) :
		EOS_Sessions_AttributeData()
	{
		ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		ValueType = EOS_ESessionAttributeType::EOS_SAT_Double;
		Value.AsDouble = InValue;
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
	}

	FAttributeOptions(const char* InKey, int32 InValue) :
		EOS_Sessions_AttributeData()
	{
		ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
		Value.AsInt64 = InValue;
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
	}

	FAttributeOptions(const char* InKey, const FVariantData& InValue) :
		EOS_Sessions_AttributeData()
	{
		ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;

		switch (InValue.GetType())
		{
			case EOnlineKeyValuePairDataType::Int32:
			{
				ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
				int32 RawValue = 0;
				InValue.GetValue(RawValue);
				Value.AsInt64 = RawValue;
				break;
			}
			case EOnlineKeyValuePairDataType::UInt32:
			{
				ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
				uint32 RawValue = 0;
				InValue.GetValue(RawValue);
				Value.AsInt64 = RawValue;
				break;
			}
			case EOnlineKeyValuePairDataType::Int64:
			{
				ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
				int64 RawValue = 0;
				InValue.GetValue(RawValue);
				Value.AsInt64 = RawValue;
				break;
			}
			case EOnlineKeyValuePairDataType::Bool:
			{
				ValueType = EOS_ESessionAttributeType::EOS_SAT_Boolean;
				bool RawValue = false;
				InValue.GetValue(RawValue);
				Value.AsBool = RawValue ? EOS_TRUE : EOS_FALSE;
				break;
			}
			case EOnlineKeyValuePairDataType::Double:
			{
				ValueType = EOS_ESessionAttributeType::EOS_SAT_Double;
				double RawValue = 0.0;
				InValue.GetValue(RawValue);
				Value.AsDouble = RawValue;
				break;
			}
			case EOnlineKeyValuePairDataType::Float:
			{
				ValueType = EOS_ESessionAttributeType::EOS_SAT_Double;
				float RawValue = 0.0f;
				InValue.GetValue(RawValue);
				Value.AsDouble = RawValue;
				break;
			}
			case EOnlineKeyValuePairDataType::String:
			{
				ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
				Value.AsUtf8 = ValueAnsi;
				Key = KeyAnsi;
				FString OutString;
				InValue.GetValue(OutString);
				FCStringAnsi::Strncpy(ValueAnsi, TCHAR_TO_UTF8(*OutString), EOS_OSS_STRING_BUFFER_LENGTH);
				break;
			}
		}
		Key = KeyAnsi;
		FCStringAnsi::Strncpy(KeyAnsi, InKey, EOS_OSS_STRING_BUFFER_LENGTH);
	}
};

// TNamedSessionOptions template struct (from EIK)
template<typename BaseStruct>
struct TNamedSessionOptions :
	public BaseStruct
{
	char SessionNameAnsi[EOS_OSS_STRING_BUFFER_LENGTH];

	TNamedSessionOptions(const char* InSessionNameAnsi)
		: BaseStruct()
	{
		FCStringAnsi::Strncpy(SessionNameAnsi, InSessionNameAnsi, EOS_OSS_STRING_BUFFER_LENGTH);
		this->SessionName = SessionNameAnsi;
	}
};

// FSessionCreateOptions struct (from EIK)
struct FSessionCreateOptions :
	public TNamedSessionOptions<EOS_Sessions_CreateSessionModificationOptions>
{
	FSessionCreateOptions(const char* InSessionNameAnsi) :
		TNamedSessionOptions<EOS_Sessions_CreateSessionModificationOptions>(InSessionNameAnsi)
	{
		ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST;
		BucketId = BucketIdAnsi;
	}
};

// FJoinSessionOptions struct (from EIK)
struct FJoinSessionOptions :
	public TNamedSessionOptions<EOS_Sessions_JoinSessionOptions>
{
	FJoinSessionOptions(const char* InSessionNameAnsi) :
		TNamedSessionOptions<EOS_Sessions_JoinSessionOptions>(InSessionNameAnsi)
	{
		ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
	}
};

// FSessionStartOptions struct (from EIK)
struct FSessionStartOptions :
	public TNamedSessionOptions<EOS_Sessions_StartSessionOptions>
{
	FSessionStartOptions(const char* InSessionNameAnsi) :
		TNamedSessionOptions<EOS_Sessions_StartSessionOptions>(InSessionNameAnsi)
	{
		ApiVersion = EOS_SESSIONS_STARTSESSION_API_LATEST;
	}
};

// FSessionUpdateOptions struct (from EIK)
struct FSessionUpdateOptions :
	public TNamedSessionOptions<EOS_Sessions_UpdateSessionModificationOptions>
{
	FSessionUpdateOptions(const char* InSessionNameAnsi) :
		TNamedSessionOptions<EOS_Sessions_UpdateSessionModificationOptions>(InSessionNameAnsi)
	{
		ApiVersion = EOS_SESSIONS_UPDATESESSIONMODIFICATION_API_LATEST;
	}
};

// FSessionEndOptions struct (from EIK)
struct FSessionEndOptions :
	public TNamedSessionOptions<EOS_Sessions_EndSessionOptions>
{
	FSessionEndOptions(const char* InSessionNameAnsi) :
		TNamedSessionOptions<EOS_Sessions_EndSessionOptions>(InSessionNameAnsi)
	{
		ApiVersion = EOS_SESSIONS_ENDSESSION_API_LATEST;
	}
};

// FSessionDestroyOptions struct (from EIK)
struct FSessionDestroyOptions :
	public TNamedSessionOptions<EOS_Sessions_DestroySessionOptions>
{
	FSessionDestroyOptions(const char* InSessionNameAnsi) :
		TNamedSessionOptions<EOS_Sessions_DestroySessionOptions>(InSessionNameAnsi)
	{
		ApiVersion = EOS_SESSIONS_DESTROYSESSION_API_LATEST;
	}
};

// FJoinSessionCallback typedef (from EIK)
#if ENGINE_MAJOR_VERSION == 5
typedef TEOSCallback<EOS_Sessions_OnJoinSessionCallback, EOS_Sessions_JoinSessionCallbackInfo, FOnlineSessionEOSKit> FJoinSessionCallback;
#else
typedef TEOSCallback<EOS_Sessions_OnJoinSessionCallback, EOS_Sessions_JoinSessionCallbackInfo> FJoinSessionCallback;
#endif

// FSessionSearchEOS wrapper (from EIK)
struct FSessionSearchEOS
{
	EOS_HSessionSearch SearchHandle;

	FSessionSearchEOS(EOS_HSessionSearch InSearchHandle)
		: SearchHandle(InSearchHandle)
	{
	}

	~FSessionSearchEOS()
	{
		if (SearchHandle != nullptr)
		{
			EOS_SessionSearch_Release(SearchHandle);
		}
	}
};

// Forward declare FLANSession (from EIK)
class FLANSession;

// FSessionSearchHandleWrapper (moved from local scope to be accessible everywhere)
struct FSessionSearchHandleWrapper
{
	EOS_HSessionSearch SearchHandle;
	FSessionSearchHandleWrapper(EOS_HSessionSearch InHandle) : SearchHandle(InHandle) {}
	~FSessionSearchHandleWrapper() { if (SearchHandle) EOS_SessionSearch_Release(SearchHandle); }
};

// Helper function to get ProductUserId from UEOSKitSubsystem
static EOS_ProductUserId GetProductUserIdFromSubsystem(int32 LocalUserNum)
{
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				if (UEOSKitSubsystem* EOSKitSubsystemPtr = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitSubsystem>())
				{
					return EOSKitSubsystemPtr->GetProductUserId(LocalUserNum);
				}
			}
		}
	}
	return nullptr;
}

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

// Callback for EOS_Sessions_EndSession (from EIK)
#if ENGINE_MAJOR_VERSION == 5
typedef TEOSCallback<EOS_Sessions_OnEndSessionCallback, EOS_Sessions_EndSessionCallbackInfo, FOnlineSessionEOSKit> FEndSessionCallback;
#else
typedef TEOSCallback<EOS_Sessions_OnEndSessionCallback, EOS_Sessions_EndSessionCallbackInfo> FEndSessionCallback;
#endif

// Callback for EOS_Sessions_DestroySession (from EIK)
#if ENGINE_MAJOR_VERSION == 5
typedef TEOSCallback<EOS_Sessions_OnDestroySessionCallback, EOS_Sessions_DestroySessionCallbackInfo, FOnlineSessionEOSKit> FDestroySessionCallback;
#else
typedef TEOSCallback<EOS_Sessions_OnDestroySessionCallback, EOS_Sessions_DestroySessionCallbackInfo> FDestroySessionCallback;
#endif

// Callback for EOS_Sessions_UpdateSession
#if ENGINE_MAJOR_VERSION == 5
typedef TEOSCallback<EOS_Sessions_OnUpdateSessionCallback, EOS_Sessions_UpdateSessionCallbackInfo, FOnlineSessionEOSKit> FUpdateSessionCallback;
#else
typedef TEOSCallback<EOS_Sessions_OnUpdateSessionCallback, EOS_Sessions_UpdateSessionCallbackInfo> FUpdateSessionCallback;
#endif

// Callback for session invite notifications (from EIK)
#if ENGINE_MAJOR_VERSION == 5
typedef TEOSCallback<EOS_Sessions_OnSessionInviteReceivedCallback, EOS_Sessions_SessionInviteReceivedCallbackInfo, FOnlineSessionEOSKit> FSessionInviteReceivedCallback;
typedef TEOSCallback<EOS_Sessions_OnSessionInviteAcceptedCallback, EOS_Sessions_SessionInviteAcceptedCallbackInfo, FOnlineSessionEOSKit> FSessionInviteAcceptedCallback;
#else
typedef TEOSCallback<EOS_Sessions_OnSessionInviteReceivedCallback, EOS_Sessions_SessionInviteReceivedCallbackInfo> FSessionInviteReceivedCallback;
typedef TEOSCallback<EOS_Sessions_OnSessionInviteAcceptedCallback, EOS_Sessions_SessionInviteAcceptedCallbackInfo> FSessionInviteAcceptedCallback;
#endif

FOnlineSessionEOSKit::FOnlineSessionEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, SessionsHandle(nullptr)
	, LobbyHandle(nullptr)
	, SessionSearchStartInSeconds(0.0)
	, SessionInviteAcceptedId(0)
	, bIsDedicatedServer(false)
	, bIsUsingP2PSockets(false)
	, LobbyUpdateReceivedId(0)
	, LobbyMemberUpdateReceivedId(0)
	, LobbyMemberStatusReceivedId(0)
	, LobbyInviteAcceptedId(0)
	, JoinLobbyAcceptedId(0)
#if PLATFORM_WINDOWS
	, LeaveLobbyRequestId(0)
#endif
{
	// Initialize BucketIdAnsi (like EIK does)
	FString BucketId = TEXT("DefaultBucket");
	if (EOSKitSubsystem)
	{
		SessionsHandle = EOSKitSubsystem->SessionsHandle;
		if (EOSKitSubsystem->EOSPlatformHandle)
		{
			LobbyHandle = EOS_Platform_GetLobbyInterface(*EOSKitSubsystem->EOSPlatformHandle);
			RegisterLobbyNotifications();
		}
	}
	FCStringAnsi::Strncpy(BucketIdAnsi, TCHAR_TO_UTF8(*BucketId), EOS_OSS_STRING_BUFFER_LENGTH);
	
	// Call Init with default bucket ID
	Init(BucketId);
	
	// Set dedicated server and P2P socket flags (from EIK)
	bIsDedicatedServer = IsRunningDedicatedServer();
	bIsUsingP2PSockets = false;

	if (!bIsDedicatedServer)
	{
		if (!GConfig->GetBool(TEXT("/Script/OnlineSubsystemEOSKit.NetDriverEOS"), TEXT("bIsUsingP2PSockets"), bIsUsingP2PSockets, GEngineIni))
		{
			// Fallback to base location
			GConfig->GetBool(TEXT("/Script/SocketSubsystemEOS.NetDriverEOSBase"), TEXT("bIsUsingP2PSockets"), bIsUsingP2PSockets, GEngineIni);
		}
	}
}

FOnlineSessionEOSKit::~FOnlineSessionEOSKit()
{
	// Remove lobby notifications
	if (LobbyHandle)
	{
		if (LobbyUpdateReceivedId != 0)
		{
			EOS_Lobby_RemoveNotifyLobbyUpdateReceived(LobbyHandle, LobbyUpdateReceivedId);
		}
		if (LobbyMemberUpdateReceivedId != 0)
		{
			EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(LobbyHandle, LobbyMemberUpdateReceivedId);
		}
		if (LobbyMemberStatusReceivedId != 0)
		{
			EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(LobbyHandle, LobbyMemberStatusReceivedId);
		}
		if (LobbyInviteAcceptedId != 0)
		{
			EOS_Lobby_RemoveNotifyLobbyInviteAccepted(LobbyHandle, LobbyInviteAcceptedId);
		}
		if (JoinLobbyAcceptedId != 0)
		{
			EOS_Lobby_RemoveNotifyJoinLobbyAccepted(LobbyHandle, JoinLobbyAcceptedId);
		}
#if PLATFORM_WINDOWS
		if (LeaveLobbyRequestId != 0)
		{
			EOS_Lobby_RemoveNotifyLeaveLobbyRequested(LobbyHandle, LeaveLobbyRequestId);
		}
#endif
	}
	
	// Remove session invite notification
	if (SessionsHandle && SessionInviteAcceptedId != 0)
	{
		EOS_Sessions_RemoveNotifySessionInviteAccepted(SessionsHandle, SessionInviteAcceptedId);
	}
}

void FOnlineSessionEOSKit::Init(const FString& InBucketId)
{
	FCStringAnsi::Strncpy(BucketIdAnsi, TCHAR_TO_UTF8(*InBucketId), EOS_OSS_STRING_BUFFER_LENGTH);

	// Note: Callback registration is deferred until after construction to avoid calling AsShared() during construction
	// The callbacks will be registered when first needed or via a separate initialization step
}

void FOnlineSessionEOSKit::RegisterSessionInviteCallbacks()
{
	if (!SessionsHandle)
	{
		return;
	}

	// Register for session invite notifications
#if ENGINE_MAJOR_VERSION == 5
	FSessionInviteReceivedCallback* SessionInviteReceivedCallbackObj = new FSessionInviteReceivedCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
	SessionInviteReceivedCallbackObj->CallbackLambda = [this](const EOS_Sessions_SessionInviteReceivedCallbackInfo* Data)
	{
		// Session invite received - can be handled here if needed
	};
	FSessionInviteAcceptedCallback* SessionInviteAcceptedCallbackObj = new FSessionInviteAcceptedCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
#else
	FSessionInviteReceivedCallback* SessionInviteReceivedCallbackObj = new FSessionInviteReceivedCallback();
	SessionInviteReceivedCallbackObj->CallbackLambda = [this](const EOS_Sessions_SessionInviteReceivedCallbackInfo* Data)
	{
		// Session invite received - can be handled here if needed
	};
	FSessionInviteAcceptedCallback* SessionInviteAcceptedCallbackObj = new FSessionInviteAcceptedCallback();
#endif
	SessionInviteAcceptedCallback = static_cast<void*>(SessionInviteAcceptedCallbackObj);
	SessionInviteAcceptedCallbackObj->CallbackLambda = [this](const EOS_Sessions_SessionInviteAcceptedCallbackInfo* Data)
	{
		if (!EOSKitSubsystem || !EOSKitSubsystem->GetIdentityInterface())
		{
			UE_LOG_ONLINE_SESSION(Warning, TEXT("Cannot accept invite due to invalid subsystem"));
			TriggerOnSessionUserInviteAcceptedDelegates(false, 0, nullptr, FOnlineSessionSearchResult());
			return;
		}

		FUniqueNetIdPtr NetId = EOSKitSubsystem->GetIdentityInterface()->GetUniquePlayerId(0);
		if (!NetId.IsValid())
		{
			FString ProductUserIdStr = ProductUserIdToString(Data->LocalUserId);
			UE_LOG_ONLINE_SESSION(Warning, TEXT("Cannot accept invite due to unknown user (%s)"), *ProductUserIdStr);
			TriggerOnSessionUserInviteAcceptedDelegates(false, 0, NetId, FOnlineSessionSearchResult());
			return;
		}
		int32 LocalUserNum = 0; // TODO: Get from NetId

		EOS_Sessions_CopySessionHandleByInviteIdOptions Options = { };
		Options.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST;
		Options.InviteId = Data->InviteId;
		EOS_HSessionDetails SessionDetails = nullptr;
		EOS_EResult Result = EOS_Sessions_CopySessionHandleByInviteId(SessionsHandle, &Options, &SessionDetails);
		if (Result == EOS_EResult::EOS_Success)
		{
			LastInviteSearch = MakeShared<FOnlineSessionSearch>();
			AddSearchResult(SessionDetails, LastInviteSearch.ToSharedRef());
			TriggerOnSessionUserInviteAcceptedDelegates(true, LocalUserNum, NetId, LastInviteSearch->SearchResults[0]);
		}
		else
		{
			UE_LOG_ONLINE_SESSION(Warning, TEXT("EOS_Sessions_CopySessionHandleByInviteId not successful. Finished with EOS_EResult %s"), ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
			TriggerOnSessionUserInviteAcceptedDelegates(false, LocalUserNum, NetId, FOnlineSessionSearchResult());
		}
	};
	
	EOS_Sessions_AddNotifySessionInviteAcceptedOptions Options = { };
	Options.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITEACCEPTED_API_LATEST;
	FSessionInviteAcceptedCallback* CallbackObj = static_cast<FSessionInviteAcceptedCallback*>(SessionInviteAcceptedCallbackObj);
	SessionInviteAcceptedId = EOS_Sessions_AddNotifySessionInviteAccepted(SessionsHandle, &Options, CallbackObj, CallbackObj->GetCallbackPtr());
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
	uint32 Result = ONLINE_FAIL;

	// Check for an existing session
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		// Check login status - use ProductUserId check like EIK does
		IOnlineIdentityPtr IdentityInterface = EOSKitSubsystem ? EOSKitSubsystem->GetIdentityInterface() : nullptr;
		ELoginStatus::Type LoginStatus = IdentityInterface ? IdentityInterface->GetLoginStatus(HostingPlayerNum) : ELoginStatus::NotLoggedIn;
		
		// Also check if we have a valid ProductUserId (like EIK's UserManager does)
		EOS_ProductUserId ProductUserId = GetProductUserIdFromSubsystem(HostingPlayerNum);
		bool bHasValidProductUserId = EOS_ProductUserId_IsValid(ProductUserId) != EOS_FALSE;
		
		if (bIsDedicatedServer || LoginStatus >= ELoginStatus::UsingLocalProfile || bHasValidProductUserId)
		{
			// Create a new session and deep copy the game settings
			Session = AddNamedSession(SessionName, NewSessionSettings);
			check(Session);
			Session->SessionState = EOnlineSessionState::Creating;

			if (IdentityInterface)
			{
				Session->OwningUserId = IdentityInterface->GetUniquePlayerId(HostingPlayerNum);
				Session->OwningUserName = IdentityInterface->GetPlayerNickname(HostingPlayerNum);
			}

			if (bIsDedicatedServer || (Session->OwningUserId.IsValid() && Session->OwningUserId->IsValid()) || bHasValidProductUserId)
			{
				// RegisterPlayer will update these values for the local player
	Session->NumOpenPrivateConnections = NewSessionSettings.NumPrivateConnections;
	Session->NumOpenPublicConnections = NewSessionSettings.NumPublicConnections;

				Session->HostingPlayerNum = HostingPlayerNum;

				// Unique identifier of this build for compatibility
	Session->SessionSettings.BuildUniqueId = GetBuildUniqueId();

				// Create Internet or LAN match
				if (!NewSessionSettings.bIsLANMatch)
				{
					if (Session->SessionSettings.bUseLobbiesIfAvailable)
					{
						Result = CreateLobbySession(HostingPlayerNum, Session);
					}
					else
					{
						Result = CreateEOSSessionInternal(HostingPlayerNum, Session);
					}
				}
				else
				{
					Result = CreateLANSession(HostingPlayerNum, Session);
				}
			}
			else
			{
				UE_LOG_ONLINE_SESSION(Warning, TEXT("Cannot create session '%s': invalid user (%d)."), *SessionName.ToString(), HostingPlayerNum);
			}

			if (Result != ONLINE_IO_PENDING)
			{
				// Set the game state as pending (not started)
				Session->SessionState = EOnlineSessionState::Pending;

				if (Result != ONLINE_SUCCESS)
				{
					// Clean up the session info so we don't get into a confused state
		RemoveNamedSession(SessionName);
	}
	else
	{
					RegisterLocalPlayers(Session);
				}
			}
		}
		else
		{
			UE_LOG_ONLINE_SESSION(Warning, TEXT("Cannot create session '%s': user not logged in (%d)."), *SessionName.ToString(), HostingPlayerNum);
		}
	}
	else
	{
		UE_LOG_ONLINE_SESSION(Warning, TEXT("Cannot create session '%s': session already exists."), *SessionName.ToString());
	}

	if (Result != ONLINE_IO_PENDING)
	{
		AsyncTask(ENamedThreads::GameThread, [this, SessionName, Result]()
			{
				TriggerOnCreateSessionCompleteDelegates(SessionName, Result == ONLINE_SUCCESS);
			});
	}

	return true;
}

bool FOnlineSessionEOSKit::CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	return CreateSession(0, SessionName, NewSessionSettings);
}

bool FOnlineSessionEOSKit::StartSession(FName SessionName)
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::StartSession: Starting session '%s'"), *SessionName.ToString());
	
	// Validate session name
	if (SessionName.IsNone() || SessionName.ToString().IsEmpty())
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::StartSession: Invalid session name (None or empty)"));
		TriggerOnStartSessionCompleteDelegates(SessionName, false);
		return false;
	}
	
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!Session)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: Session '%s' not found"), *SessionName.ToString());
		TriggerOnStartSessionCompleteDelegates(SessionName, false);
		return false;
	}

	// Log current session state for debugging
	const TCHAR* StateName = TEXT("Unknown");
	switch (Session->SessionState)
	{
		case EOnlineSessionState::NoSession: StateName = TEXT("NoSession"); break;
		case EOnlineSessionState::Creating: StateName = TEXT("Creating"); break;
		case EOnlineSessionState::Pending: StateName = TEXT("Pending"); break;
		case EOnlineSessionState::Starting: StateName = TEXT("Starting"); break;
		case EOnlineSessionState::InProgress: StateName = TEXT("InProgress"); break;
		case EOnlineSessionState::Ending: StateName = TEXT("Ending"); break;
		case EOnlineSessionState::Ended: StateName = TEXT("Ended"); break;
		case EOnlineSessionState::Destroying: StateName = TEXT("Destroying"); break;
	}
	UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::StartSession: Session '%s' current state: %d (%s)"), 
		*SessionName.ToString(), static_cast<int32>(Session->SessionState), StateName);

	// Allow starting if session is in Pending, Creating, or NoSession (in case it wasn't properly initialized)
	// Also allow if already InProgress (idempotent - just return success)
	if (Session->SessionState == EOnlineSessionState::InProgress)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: Session '%s' is already started (InProgress)"), *SessionName.ToString());
		TriggerOnStartSessionCompleteDelegates(SessionName, true);
		return true;
	}
	
	if (Session->SessionState != EOnlineSessionState::Pending && 
		Session->SessionState != EOnlineSessionState::Creating &&
		Session->SessionState != EOnlineSessionState::NoSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: Session '%s' is in wrong state (%d - %s). Expected: Pending(2), Creating(1), or NoSession(0)"), 
			*SessionName.ToString(), static_cast<int32>(Session->SessionState), StateName);
		TriggerOnStartSessionCompleteDelegates(SessionName, false);
		return false;
	}

	// If session is in NoSession state, set it to Creating first
	if (Session->SessionState == EOnlineSessionState::NoSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartSession: Session '%s' was in NoSession state, setting to Creating"), *SessionName.ToString());
		Session->SessionState = EOnlineSessionState::Creating;
	}

	// Check if using lobbies (like EIK does)
	if (!Session->SessionSettings.bIsLANMatch)
	{
		if (Session->SessionSettings.bUseLobbiesIfAvailable)
		{
			uint32 Result = StartLobbySession(Session);
			return (Result == ONLINE_SUCCESS || Result == ONLINE_IO_PENDING);
		}
		else
		{
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
		}
	}
	else
	{
		// LAN session - just mark as started
	Session->SessionState = EOnlineSessionState::InProgress;
	TriggerOnStartSessionCompleteDelegates(SessionName, true);
	return true;
	}
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
	check(Session != nullptr);

	EOS_HSessionModification SessionModHandle = nullptr;

	FSessionCreateOptions Options(TCHAR_TO_UTF8(*Session->SessionName.ToString()));
	Options.MaxPlayers = Session->SessionSettings.NumPrivateConnections + Session->SessionSettings.NumPublicConnections;

	EOS_ProductUserId LocalProductUserId = nullptr;
	if (EOSKitSubsystem)
	{
		// Try to get ProductUserId from the subsystem's GetProductUserId method
		// This requires getting the UEOSKitSubsystem from the game instance
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.World() && Context.World()->GetGameInstance())
				{
					if (UEOSKitSubsystem* EOSKitSubsystemPtr = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitSubsystem>())
					{
						LocalProductUserId = GetProductUserIdFromSubsystem(HostingPlayerNum);
			break;
					}
				}
			}
		}
		
		// Fallback: Get from identity interface
		if (!EOS_ProductUserId_IsValid(LocalProductUserId) && EOSKitSubsystem->GetIdentityInterface())
		{
			FUniqueNetIdPtr LocalUserId = EOSKitSubsystem->GetIdentityInterface()->GetUniquePlayerId(HostingPlayerNum);
			if (LocalUserId.IsValid())
			{
				FString UserIdStr = LocalUserId->ToString();
				// Handle "EpicAccountId|ProductUserId" format
				if (UserIdStr.Contains(TEXT("|")))
				{
					TArray<FString> Parts;
					UserIdStr.ParseIntoArray(Parts, TEXT("|"), true);
					if (Parts.Num() >= 2)
					{
						LocalProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Parts[1]));
					}
		}
		else
		{
					LocalProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserIdStr));
				}
			}
		}
	}
	if (!EOS_ProductUserId_IsValid(LocalProductUserId))
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("CreateEOSSessionInternal: Invalid ProductUserId for HostingPlayerNum %d"), HostingPlayerNum);
		return ONLINE_FAIL;
	}
	Options.LocalUserId = LocalProductUserId;
	
	Options.bPresenceEnabled = (Session->SessionSettings.bUsesPresence ||
		Session->SessionSettings.bAllowJoinViaPresence ||
		Session->SessionSettings.bAllowJoinViaPresenceFriendsOnly ||
		Session->SessionSettings.bAllowInvites) ? EOS_TRUE : EOS_FALSE;
	if (Session->SessionSettings.Get("SANCTIONENABLED", Options.bSanctionsEnabled))
	{
		UE_LOG(LogOnline, Log, TEXT("Sanctions Enabled: %s"), Options.bSanctionsEnabled ? TEXT("True") : TEXT("False"));
	}
	else
	{
		UE_LOG(LogOnline, Log, TEXT("Sanctions Enabled not found, defaulting to false"));
		Options.bSanctionsEnabled = false;
	}
	EOS_EResult ResultCode = EOS_Sessions_CreateSessionModification(SessionsHandle, &Options, &SessionModHandle);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("EOS_Sessions_CreateSessionModification() failed with EOS result code (%s)"), ANSI_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
		return ONLINE_FAIL;
	}

	Session->SessionState = EOnlineSessionState::Creating;
	Session->bHosting = true;

	FString HostAddr;
	// CRITICAL: Only advertise EOS address if:
	// 1. Not a dedicated server
	// 2. Config says to use P2P sockets
	// 3. EOS NetDriver is actually available (will be used, not fallback to IP)
	// This prevents advertising EOS address when host will actually use IpNetDriver fallback
	bool bShouldUseEOSAddress = false;
	if (!bIsDedicatedServer && bIsUsingP2PSockets)
	{
		// CRITICAL: Check if EOS NetDriver is actually available AND will be used (not fallback to IP)
		// This must match the same check that InitListen uses (IsAvailable())
		// We need to verify:
		// 1. UNetDriverEOS class exists
		// 2. EOS socket subsystem exists AND is fully initialized (has valid ProductUserId)
		// This ensures the host will use EOS NetDriver, not fallback to IpNetDriver
		if (UNetDriverEOS::StaticClass())
		{
			const FName EOSSubsystemName(TEXT("EOS"));
			ISocketSubsystem* EOSSocketSub = ISocketSubsystem::Get(EOSSubsystemName);
			
			if (EOSSocketSub != nullptr)
			{
				// CRITICAL: Check if socket subsystem is fully initialized (has valid ProductUserId)
				// This matches what IsAvailable() and InitListen check
				// Try to get local bind address - this will fail if ProductUserId is invalid
				// Note: TSharedRef is always valid, we only need to check the address itself
				TSharedRef<FInternetAddr> TestAddr = EOSSocketSub->GetLocalBindAddr(*GLog);
				if (TestAddr->IsValid())
				{
					// Socket subsystem is fully initialized and can be used
					bShouldUseEOSAddress = true;
					UE_LOG_ONLINE_SESSION(Log, TEXT("CreateEOSSessionInternal: ✅ EOS NetDriver is available and fully initialized - will advertise EOS address"));
				}
				else
				{
					UE_LOG_ONLINE_SESSION(Warning, TEXT("CreateEOSSessionInternal: ❌ EOS socket subsystem exists but not fully initialized (invalid ProductUserId) - will use IP fallback address"));
				}
			}
			else
			{
				UE_LOG_ONLINE_SESSION(Warning, TEXT("CreateEOSSessionInternal: ❌ EOS socket subsystem not found - will use IP fallback address"));
			}
		}
		else
		{
			UE_LOG_ONLINE_SESSION(Warning, TEXT("CreateEOSSessionInternal: ❌ UNetDriverEOS class not found - will use IP fallback address"));
		}
	}
	
	if (bShouldUseEOSAddress)
	{
		// Because some platforms remap ports, we will use the ID of the name of the net driver to be our port instead
		FName NetDriverName = NAME_GameNetDriver; // Use default net driver name
		// Try to find the EOS net driver from any world context
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (UWorld* World = Context.World())
				{
					if (UNetDriver* NetDriver = World->GetNetDriver())
					{
						if (UNetDriverEOS* EOSNetDriver = Cast<UNetDriverEOS>(NetDriver))
						{
							NetDriverName = EOSNetDriver->NetDriverName;
							UE_LOG_ONLINE_SESSION(Log, TEXT("CreateEOSSessionInternal: Found active EOS NetDriver with name: %s"), *NetDriverName.ToString());
							break;
						}
					}
				}
			}
		}
		FString ProductUserIdStr = ProductUserIdToString(Options.LocalUserId);
		FInternetAddrEOS TempAddr(ProductUserIdStr, NetDriverName.ToString(), GetTypeHash(NetDriverName.ToString()));
		HostAddr = TempAddr.ToString(true);
		char HostAddrAnsi[EOS_OSS_STRING_BUFFER_LENGTH];
		FCStringAnsi::Strncpy(HostAddrAnsi, TCHAR_TO_UTF8(*HostAddr), EOS_OSS_STRING_BUFFER_LENGTH);

		EOS_SessionModification_SetHostAddressOptions HostOptions = { };
			HostOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETHOSTADDRESS_API_LATEST;
		// Expect URLs to look like "EOS:PUID:SocketName:Channel" and channel can be optional
		HostOptions.HostAddress = HostAddrAnsi;
			EOS_EResult HostResult = EOS_SessionModification_SetHostAddress(SessionModHandle, &HostOptions);
		UE_LOG_ONLINE_SESSION(Log, TEXT("CreateEOSSessionInternal: ✅ Advertising EOS address: %s (host will use UNetDriverEOS)"), *HostAddr);
		UE_LOG_ONLINE_SESSION(Log, TEXT("EOS_SessionModification_SetHostAddress(%s) returned (%s)"), *HostAddr, ANSI_TO_TCHAR(EOS_EResult_ToString(HostResult)));
	}
	else
	{
		// Fallback to IP address (host will use IpNetDriver)
		HostAddr = TEXT("127.0.0.1");
		UE_LOG_ONLINE_SESSION(Log, TEXT("CreateEOSSessionInternal: ⚠️ Using IP fallback address: %s (host will use IpNetDriver)"), *HostAddr);
	}
	Session->SessionInfo = MakeShareable(new FOnlineSessionInfoEOSKit(HostAddr));

	FName SessionName = Session->SessionName;
#if ENGINE_MAJOR_VERSION == 5
	FUpdateSessionCallback* CallbackObj = new FUpdateSessionCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
#else
	FUpdateSessionCallback* CallbackObj = new FUpdateSessionCallback();
#endif
	CallbackObj->CallbackLambda = [this, SessionName](const EOS_Sessions_UpdateSessionCallbackInfo* Data)
	{
		bool bWasSuccessful = false;

		FNamedOnlineSession* Session = GetNamedSession(SessionName);
		if (Session)
		{
			bWasSuccessful = Data->ResultCode == EOS_EResult::EOS_Success || Data->ResultCode == EOS_EResult::EOS_Sessions_OutOfSync;
			if (bWasSuccessful)
			{
				TSharedPtr<FOnlineSessionInfoEOSKit> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoEOSKit>(Session->SessionInfo);
				if (SessionInfo.IsValid())
				{
					SessionInfo->SetSessionId(UTF8_TO_TCHAR(Data->SessionId));
				}

				Session->SessionState = EOnlineSessionState::Pending;
				BeginSessionAnalytics(Session);

				RegisterLocalPlayers(Session);
			}
			else
			{
				UE_LOG_ONLINE_SESSION(Error, TEXT("EOS_Sessions_UpdateSession() failed with EOS result code (%s)"), ANSI_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));

				Session->SessionState = EOnlineSessionState::NoSession;

				RemoveNamedSession(SessionName);
			}
		}

		TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);
	};

	return SharedSessionUpdate(SessionModHandle, Session, static_cast<void*>(CallbackObj));
}

void FOnlineSessionEOSKit::HandleCreateSessionCallback(FName SessionName, const EOS_Sessions_UpdateSessionCallbackInfo* Data)
{
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
}

void FOnlineSessionEOSKit::RegisterLocalPlayers(FNamedOnlineSession* Session)
{
	// EMPTY - Just like EIK does!
	// For P2P sessions, EOS automatically registers the host when StartSession is called
	// Manual RegisterPlayer calls fail with P2P policy (403: missing matchmaking:managePlayers)
	// 
	// If you're using Matchmaking policy (not P2P), you would need to implement registration here
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
	
	if (!SessionsHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::FindSessions: SessionsHandle is null"));
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}
	
	// Check if using lobbies (exactly like EIK does)
	if (!SearchSettings->bIsLanQuery)
	{
		bool bUseLobbiesIfAvailable = false;
		
		// Try to get SEARCH_LOBBIES from QuerySettings
		bool bHasSearchLobbies = SearchSettings->QuerySettings.Get(SEARCH_LOBBIES, bUseLobbiesIfAvailable);
		
		// Also check SearchParams directly as a fallback
		if (!bHasSearchLobbies)
		{
			const FOnlineSessionSearchParam* SearchParam = SearchSettings->QuerySettings.SearchParams.Find(SEARCH_LOBBIES);
			if (SearchParam)
			{
				// Extract boolean value from SearchParam
				bool bValue = false;
				if (SearchParam->Data.GetType() == EOnlineKeyValuePairDataType::Bool)
				{
					SearchParam->Data.GetValue(bValue);
					bUseLobbiesIfAvailable = bValue;
					bHasSearchLobbies = true;
				}
			}
		}
		
		if (bHasSearchLobbies && bUseLobbiesIfAvailable)
		{
			UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Searching for LOBBIES (P2P via EOS Lobby API)"));
			uint32 Return = FindLobbySession(SearchingPlayerNum, SearchSettings);
			if (Return == ONLINE_IO_PENDING)
			{
				SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
			}
			return Return == ONLINE_SUCCESS || Return == ONLINE_IO_PENDING;
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: NOT searching for lobbies (bHasSearchLobbies=%d, bUseLobbiesIfAvailable=%d), using regular session search"), bHasSearchLobbies ? 1 : 0, bUseLobbiesIfAvailable ? 1 : 0);
			// Continue to regular EOS session search below
		}
	}
	else
	{
		// LAN search - not implemented yet
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::FindSessions: LAN search not implemented"));
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
	
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Using BucketId='%s' (must match session creation)"), *BucketId);
	
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
	
	EOS_EResult SetParamResult = EOS_SessionSearch_SetParameter(SearchHandle, &BucketParam);
	if (SetParamResult != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::FindSessions: Failed to set BucketId parameter: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(SetParamResult)));
		EOS_SessionSearch_Release(SearchHandle);
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}
	
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
	
	// Store search handle to keep it alive (like EIK does with CurrentSearchHandle)
	// This ensures SessionDetailsHandles remain valid
	// Wrap in a struct since EOS_HSessionSearch is a handle type
	TSharedPtr<FSessionSearchHandleWrapper> SharedSearchHandle = MakeShareable(new FSessionSearchHandleWrapper(SearchHandle));
	
	FFindSessionsCallback* CallbackObj = new FFindSessionsCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
	CallbackObj->CallbackLambda = [this, SearchingPlayerNum, SharedSearchHandle, LocalUserId, SearchSettings](const EOS_SessionSearch_FindCallbackInfo* Data)
	{
		bool bWasSuccessful = (Data->ResultCode == EOS_EResult::EOS_Success);
		
		if (bWasSuccessful)
		{
			// Get number of results
			EOS_SessionSearch_GetSearchResultCountOptions CountOptions = {};
			CountOptions.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
			int32 NumResults = EOS_SessionSearch_GetSearchResultCount(SharedSearchHandle->SearchHandle, &CountOptions);
			
			UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: ✅ Found %d session(s)"), NumResults);
			
			// Copy each result
			EOS_SessionSearch_CopySearchResultByIndexOptions IndexOptions = {};
			IndexOptions.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
			
			for (int32 Index = 0; Index < NumResults; Index++)
			{
				EOS_HSessionDetails SessionDetailsHandle = nullptr;
				IndexOptions.SessionIndex = Index;
				
				EOS_EResult CopyResult = EOS_SessionSearch_CopySearchResultByIndex(SharedSearchHandle->SearchHandle, &IndexOptions, &SessionDetailsHandle);
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
						
						// CRITICAL: Set EOSAddress for P2P connections
						// Prefer HostAddress from EOS if it's already in EOS format (EOS:ProductUserId:...)
						// Otherwise, build it from OwnerUserId
						if (HostAddrStr.StartsWith(TEXT("EOS:"), ESearchCase::IgnoreCase))
						{
							// HostAddress is already in EOS format - use it directly
							NewSessionInfo->EOSAddress = HostAddrStr;
							UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::FindSessions: Using HostAddress as EOSAddress: %s"), *NewSessionInfo->EOSAddress);
						}
						else if (EOS_ProductUserId_IsValid(SessionInfo->OwnerUserId))
						{
							// Build EOS P2P address from OwnerUserId: EOS:HostProductUserId:GameNetDriver:26 (EIK format)
							char OwnerIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
							int32 BufferSize = sizeof(OwnerIdStr);
							if (EOS_ProductUserId_ToString(SessionInfo->OwnerUserId, OwnerIdStr, &BufferSize) == EOS_EResult::EOS_Success)
							{
								NewSessionInfo->EOSAddress = FString::Printf(TEXT("EOS:%s:GameNetDriver:26"), UTF8_TO_TCHAR(OwnerIdStr));
								UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::FindSessions: Built EOSAddress from OwnerUserId: %s"), *NewSessionInfo->EOSAddress);
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
						
						// CRITICAL: Set bUseLobbiesIfAvailable based on search type
						// This tells JoinSession whether to use JoinLobbySession or JoinEOSSession
						bool bWasLobbySearch = false;
						SearchSettings->QuerySettings.Get(FName(TEXT("SEARCH_LOBBIES")), bWasLobbySearch);
						Result.Session.SessionSettings.bUseLobbiesIfAvailable = bWasLobbySearch;
						UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::FindSessions: Set bUseLobbiesIfAvailable=%d for result"), bWasLobbySearch ? 1 : 0);
						
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
		
		// NOTE: SearchHandle is kept alive via SharedSearchHandle until all SessionDetailsHandles are used
		// The SessionDetailsHandles from CopySearchResultByIndex are independent handles that must be released
		// separately. SearchHandle will be released when SharedSearchHandle is destroyed.
		
		TriggerOnFindSessionsCompleteDelegates(bWasSuccessful);
	};
	
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::FindSessions: Invalid ProductUserId - cannot search"));
		EOS_SessionSearch_Release(SearchHandle);
		SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}

	char LocalUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32 LocalUserIdStrSize = sizeof(LocalUserIdStr);
	if (EOS_ProductUserId_ToString(LocalUserId, LocalUserIdStr, &LocalUserIdStrSize) == EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Searching with LocalUserId: %s"), UTF8_TO_TCHAR(LocalUserIdStr));
	}
	
	// Execute the search
	EOS_SessionSearch_FindOptions FindOptions = { };
	FindOptions.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
	FindOptions.LocalUserId = LocalUserId;
	
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Executing EOS_SessionSearch_Find..."));
	EOS_SessionSearch_Find(SearchHandle, &FindOptions, CallbackObj, CallbackObj->GetCallbackPtr());
	
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::FindSessions: Search initiated (async callback will report result)"));
	
	return true;
}

bool FOnlineSessionEOSKit::FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	return FindSessions(0, SearchSettings);
}

bool FOnlineSessionEOSKit::FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& Delegate)
{
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetIdentityInterface())
	{
	Delegate.ExecuteIfBound(0, false, FOnlineSessionSearchResult());
	return false;
	}

	// Get local user num from SearchingUserId
	int32 LocalUserNum = 0;
	FUniqueNetIdPtr LocalUserNetId = EOSKitSubsystem->GetIdentityInterface()->GetUniquePlayerId(0);
	if (LocalUserNetId.IsValid() && *LocalUserNetId == SearchingUserId)
	{
		LocalUserNum = 0;
	}
	else
	{
		// Try to find the user num
		for (int32 i = 0; i < MAX_LOCAL_PLAYERS; i++)
		{
			FUniqueNetIdPtr UserId = EOSKitSubsystem->GetIdentityInterface()->GetUniquePlayerId(i);
			if (UserId.IsValid() && *UserId == SearchingUserId)
			{
				LocalUserNum = i;
				break;
			}
		}
	}

	FindEOSSessionById(LocalUserNum, SessionId, Delegate);
	return true;
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
	
	// Create Internet or LAN match
	if (!Session->SessionSettings.bIsLANMatch)
	{
		if (DesiredSession.Session.SessionInfo.IsValid())
		{
			TSharedPtr<const FOnlineSessionInfoEOSKit> SearchSessionInfo = StaticCastSharedPtr<const FOnlineSessionInfoEOSKit>(DesiredSession.Session.SessionInfo);

			FOnlineSessionInfoEOSKit* NewSessionInfo = new FOnlineSessionInfoEOSKit(*SearchSessionInfo);
			Session->SessionInfo = MakeShareable(NewSessionInfo);

			if (DesiredSession.Session.SessionSettings.bUseLobbiesIfAvailable)
			{
				uint32 JoinResult = JoinLobbySession(PlayerNum, Session, &DesiredSession.Session);
				if (JoinResult == ONLINE_IO_PENDING)
				{
					return true;
				}
				else
				{
					RemoveNamedSession(SessionName);
					TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
					return false;
				}
			}
			else
			{
				uint32 JoinResult = JoinEOSSession(PlayerNum, Session, &DesiredSession.Session);
				if (JoinResult != ONLINE_IO_PENDING)
				{
					if (JoinResult != ONLINE_SUCCESS)
					{
						RemoveNamedSession(SessionName);
						TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
						return false;
					}
					else
					{
						TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::Success);
						return true;
					}
				}
				return true;
			}
		}
		else
		{
			UE_LOG_ONLINE_SESSION(Warning, TEXT("Invalid session info on search result"), *SessionName.ToString());
			RemoveNamedSession(SessionName);
			TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
			return false;
		}
	}
	else
	{
		FOnlineSessionInfoEOSKit* NewSessionInfo = new FOnlineSessionInfoEOSKit();
		Session->SessionInfo = MakeShareable(NewSessionInfo);
		
		uint32 JoinResult = JoinLANSession(PlayerNum, Session, &DesiredSession.Session);
		if (JoinResult != ONLINE_IO_PENDING)
		{
			if (JoinResult != ONLINE_SUCCESS)
			{
				RemoveNamedSession(SessionName);
				TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
				return false;
			}
			else
			{
				TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::Success);
				return true;
			}
		}
		return true;
	}
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
	
	// Fallback to IP address (for LAN or non-EOS sessions, or when EOSAddress is missing)
	if (!SessionInfo->HostAddr.IsEmpty())
	{
	ConnectInfo = SessionInfo->HostAddr;
	if (!ConnectInfo.Contains(TEXT(":")))
	{
		ConnectInfo = FString::Printf(TEXT("%s:7777"), *SessionInfo->HostAddr);
		}
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: ✅ Using IP address (fallback): %s"), *ConnectInfo);
		return true;
	}
	
	// Last resort: If both are empty but we have a session, try to construct from session ID or use default
	// This helps with local testing when EOSAddress isn't set
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: Both EOSAddress and HostAddr are empty, using default 127.0.0.1:7777"));
	ConnectInfo = TEXT("127.0.0.1:7777");
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
	if (!SessionInfo->HostAddr.IsEmpty())
	{
	ConnectInfo = SessionInfo->HostAddr;
	
	// If no port specified, add default game port
	if (!ConnectInfo.Contains(TEXT(":")))
	{
		ConnectInfo = FString::Printf(TEXT("%s:7777"), *SessionInfo->HostAddr);
	}
	
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: ✅ Using IP address from SearchResult (fallback): %s"), *ConnectInfo);
		return true;
	}
	
	// Last resort: Default to localhost for testing
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::GetResolvedConnectString: Both EOSAddress and HostAddr are empty in SearchResult, using default 127.0.0.1:7777"));
	ConnectInfo = TEXT("127.0.0.1:7777");
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
			else if (Session->NumOpenPrivateConnections > 0)
			{
				Session->NumOpenPrivateConnections--;
				UE_LOG_ONLINE(Log, TEXT("RegisterPlayers: Decremented open slots - now %d/%d"), 
					Session->NumOpenPrivateConnections, Session->SessionSettings.NumPrivateConnections);
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

void FOnlineSessionEOSKit::TravelToSession(FName SessionName)
{
	// Resolve connect string and travel (mirrors EIK behavior)
	FString JoinAddress;
	if (!GetResolvedConnectString(SessionName, JoinAddress, NAME_GamePort) || JoinAddress.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::TravelToSession: Could not resolve connect string"));
		return;
	}

	// If dedicated, rewrite port from SessionSettings (EIK behavior)
	if (FNamedOnlineSession* Session = GetNamedSession(SessionName))
	{
		if (Session->SessionSettings.bIsDedicated)
		{
			FString PortInfo = TEXT("7777");
			if (Session->SessionSettings.Settings.Contains(FName(TEXT("PortInfo"))))
			{
				Session->SessionSettings.Get(FName(TEXT("PortInfo")), PortInfo);
			}

			TArray<FString> IpPortArray;
			JoinAddress.ParseIntoArray(IpPortArray, TEXT(":"), true);
			if (IpPortArray.Num() >= 1)
			{
				const FString IpAddress = IpPortArray[0];
				JoinAddress = IpAddress + TEXT(":") + PortInfo;
				UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::TravelToSession: Dedicated session, rewrote JoinAddress to %s"), *JoinAddress);
			}
		}
	}

	if (!GEngine)
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::TravelToSession: GEngine null"));
		return;
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (UWorld* World = Context.World())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				// CRITICAL: For EOS P2P addresses, ensure no map path is appended
				// The EOS address format is: EOS:ProductUserId:SocketName:Channel
				// Unreal Engine should NOT append the current map path to EOS URLs
				// Use TRAVEL_Absolute to prevent any automatic map path appending
				UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::TravelToSession: ClientTravel to %s (EOS P2P - no map path)"), *JoinAddress);
				
				// Ensure the URL is clean - remove any trailing map paths that might have been added
				FString CleanAddress = JoinAddress;
				int32 MapPathIndex = CleanAddress.Find(TEXT("/Game/"), ESearchCase::CaseSensitive);
				if (MapPathIndex != INDEX_NONE)
				{
					CleanAddress = CleanAddress.Left(MapPathIndex);
					UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::TravelToSession: Removed map path from EOS address, using: %s"), *CleanAddress);
				}
				
				PC->ClientTravel(CleanAddress, TRAVEL_Absolute);
				return;
			}
		}
	}

	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::TravelToSession: No PlayerController found to travel"));
}

EOS_ProductUserId FOnlineSessionEOSKit::GetProductUserIdForSession(FNamedOnlineSession* Session) const
{
	if (!Session || !EOSKitSubsystem)
	{
		return nullptr;
	}

	// Try to get ProductUserId from OwningUserId
	if (Session->OwningUserId.IsValid())
	{
		FString UserIdStr = Session->OwningUserId->ToString();
		if (UserIdStr.Contains(TEXT("|")))
		{
			TArray<FString> Parts;
			UserIdStr.ParseIntoArray(Parts, TEXT("|"), true);
			if (Parts.Num() >= 2)
			{
				return EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Parts[1]));
			}
		}
		else
		{
			return EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserIdStr));
		}
	}

	// Fallback: Get from identity interface
	if (IOnlineIdentityPtr Identity = EOSKitSubsystem->GetIdentityInterface())
	{
		FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
		if (UserId.IsValid())
		{
			FString UserIdStr = UserId->ToString();
			if (UserIdStr.Contains(TEXT("|")))
			{
				TArray<FString> Parts;
				UserIdStr.ParseIntoArray(Parts, TEXT("|"), true);
				if (Parts.Num() >= 2)
				{
					return EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Parts[1]));
				}
			}
			else
			{
				return EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserIdStr));
			}
		}
	}

	return nullptr;
}

// Helper function to resolve ProductUserId to UniqueNetId (adapted from EIK)
FUniqueNetIdPtr FOnlineSessionEOSKit::ResolveProductUserIdToUniqueNetId(EOS_ProductUserId ProductUserId) const
{
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetIdentityInterface())
	{
		return nullptr;
	}

	// Convert ProductUserId to string
	char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
	if (EOS_ProductUserId_ToString(ProductUserId, ProductUserIdStr, &ProductUserIdStrSize) != EOS_EResult::EOS_Success)
	{
		return nullptr;
	}

	FString UserIdString = UTF8_TO_TCHAR(ProductUserIdStr);
	return FUniqueNetIdString::Create(UserIdString, FName(TEXT("EOS")));
}

// Helper function to resolve multiple ProductUserIds to UniqueNetIds (adapted from EIK)
void FOnlineSessionEOSKit::ResolveProductUserIdsToUniqueNetIds(const TArray<EOS_ProductUserId>& ProductUserIds, TFunction<void(TMap<EOS_ProductUserId, FUniqueNetIdRef>)> Callback) const
{
	TMap<EOS_ProductUserId, FUniqueNetIdRef> ResolvedIds;
	
	for (EOS_ProductUserId ProductUserId : ProductUserIds)
	{
		if (FUniqueNetIdPtr ResolvedId = ResolveProductUserIdToUniqueNetId(ProductUserId))
		{
			ResolvedIds.Add(ProductUserId, ResolvedId.ToSharedRef());
		}
	}

	// Execute callback on game thread
	if (Callback)
	{
		AsyncTask(ENamedThreads::GameThread, [Callback, ResolvedIds]()
		{
			Callback(ResolvedIds);
		});
	}
}

FNamedOnlineSession* FOnlineSessionEOSKit::GetNamedSessionFromLobbyId(const FUniqueNetIdEOSLobby& LobbyId)
{
	FNamedOnlineSession* Result = nullptr;

	FScopeLock ScopeLock(&SessionLock);
	for (int32 SearchIndex = 0; SearchIndex < Sessions.Num(); SearchIndex++)
	{
		FNamedOnlineSession& Session = Sessions[SearchIndex];
		if (Session.SessionInfo.IsValid())
		{
			FOnlineSessionInfoEOSKit* SessionInfo = (FOnlineSessionInfoEOSKit*)Session.SessionInfo.Get();

			// We'll check if the session is a Lobby session before comparing the ids
			const FString LobbyIdStr = LobbyId.ToString();
			if (!Session.SessionSettings.bIsLANMatch && Session.SessionSettings.bUseLobbiesIfAvailable && SessionInfo->SessionId == LobbyIdStr)
			{
				Result = &Sessions[SearchIndex];
				break;
			}
		}
	}

	return Result;
}

FOnlineSessionSearchResult* FOnlineSessionEOSKit::GetSearchResultFromLobbyId(const FUniqueNetIdEOSLobby& LobbyId)
{
	FOnlineSessionSearchResult* Result = nullptr;

	TArray<FOnlineSessionSearchResult*> CombinedSearchResults;

	if (CurrentSessionSearch.IsValid())
	{
		for (FOnlineSessionSearchResult& SearchResult : CurrentSessionSearch->SearchResults)
		{
			CombinedSearchResults.Add(&SearchResult);
		}
	}

	if (LastInviteSearch.IsValid())
	{
		for (FOnlineSessionSearchResult& SearchResult : LastInviteSearch->SearchResults)
		{
			CombinedSearchResults.Add(&SearchResult);
		}
	}

	for (FOnlineSessionSearchResult* SearchResult : CombinedSearchResults)
	{
		const FOnlineSession& Session = SearchResult->Session;
		if (Session.SessionInfo.IsValid())
		{
			FOnlineSessionInfoEOSKit* SessionInfo = (FOnlineSessionInfoEOSKit*)Session.SessionInfo.Get();

			// We'll check if the session is a Lobby session before comparing the ids
			const FString LobbyIdStr = LobbyId.ToString();
			if (!Session.SessionSettings.bIsLANMatch && Session.SessionSettings.bUseLobbiesIfAvailable && SessionInfo->SessionId == LobbyIdStr)
			{
				Result = SearchResult;
				break;
			}
		}
	}

	return Result;
}

FOnlineSession* FOnlineSessionEOSKit::GetOnlineSessionFromLobbyId(const FUniqueNetIdEOSLobby& LobbyId)
{
	// First we try to retrieve a named session matching the given lobby id
	FOnlineSession* Result = GetNamedSessionFromLobbyId(LobbyId);

	if (!Result)
	{
		// If no named session were found with that lobby id, we look amongst the sessions in the latest search results
		if (FOnlineSessionSearchResult* SearchResult = GetSearchResultFromLobbyId(LobbyId))
		{
			Result = &SearchResult->Session;
		}
		else
		{
			UE_LOG_ONLINE_SESSION(Verbose, TEXT("[FOnlineSessionEOSKit::GetOnlineSessionFromLobbyId] Session with LobbyId [%s] not found."), *LobbyId.ToString());
		}
	}

	return Result;
}

void FOnlineSessionEOSKit::UpdateOrAddLobbyMember(const TSharedRef<const FUniqueNetIdEOSLobby>& LobbyNetId, const FUniqueNetIdRef& PlayerId)
{
	if (FNamedOnlineSession* Session = this->GetNamedSessionFromLobbyId(*LobbyNetId))
	{
		// First we add the player to the session, if it wasn't already there
		bool bWasLobbyMemberAdded = false;
		if (!Session->SessionSettings.MemberSettings.Contains(PlayerId))
		{
			bWasLobbyMemberAdded = this->AddOnlineSessionMember(Session->SessionName, PlayerId);
		}

		if (FSessionSettings* MemberSettings = Session->SessionSettings.MemberSettings.Find(PlayerId))
		{
			const FTCHARToUTF8 Utf8LobbyId(*LobbyNetId->ToString());

			EOS_Lobby_CopyLobbyDetailsHandleOptions Options = {};
			Options.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLE_API_LATEST;
			Options.LobbyId = (EOS_LobbyId)Utf8LobbyId.Get();
			Options.LocalUserId = GetProductUserIdFromSubsystem(0);

			EOS_HLobbyDetails LobbyDetailsHandle;

			EOS_EResult Result = EOS_Lobby_CopyLobbyDetailsHandle(this->LobbyHandle, &Options, &LobbyDetailsHandle);
			if (Result == EOS_EResult::EOS_Success)
			{
				TSharedRef<FLobbyDetailsEOSKit> LobbyDetails = MakeShared<FLobbyDetailsEOSKit>(LobbyDetailsHandle);

				// Extract ProductUserId from PlayerId
				FString PlayerIdStr = PlayerId->ToString();
				EOS_ProductUserId TargetProductUserId = nullptr;
				if (PlayerIdStr.Contains(TEXT("|")))
				{
					TArray<FString> Parts;
					PlayerIdStr.ParseIntoArray(Parts, TEXT("|"), true);
					if (Parts.Num() >= 2)
					{
						TargetProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Parts[1]));
					}
				}
				else
				{
					TargetProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*PlayerIdStr));
				}

				if (TargetProductUserId)
				{
					// Then we update their attributes
					this->CopyLobbyMemberAttributes(*LobbyDetails, TargetProductUserId, *MemberSettings);

					if (bWasLobbyMemberAdded)
					{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
						this->TriggerOnSessionParticipantJoinedDelegates(Session->SessionName, *PlayerId);
#else
						this->TriggerOnSessionParticipantsChangeDelegates(Session->SessionName, *PlayerId, true);
#endif
					}
					else
					{
						this->TriggerOnSessionParticipantSettingsUpdatedDelegates(Session->SessionName, *PlayerId, Session->SessionSettings);
					}
				}
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("[FOnlineSessionEOSKit::UpdateOrAddLobbyMember] EOS_Lobby_CopyLobbyDetailsHandle not successful. Finished with EOS_EResult %s"), ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
			}
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("[FOnlineSessionEOSKit::UpdateOrAddLobbyMember] UniqueNetId %s not registered in the session's member settings."), *PlayerId->ToString());
		}
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("[FOnlineSessionEOSKit::UpdateOrAddLobbyMember] Unable to retrieve session with LobbyId %s"), *LobbyNetId->ToString());
	}
}

void FOnlineSessionEOSKit::OnLobbyUpdateReceived(const EOS_LobbyId& LobbyId)
{
	const FUniqueNetIdEOSLobbyRef LobbyNetId = FUniqueNetIdEOSLobby::Create(UTF8_TO_TCHAR(LobbyId));
	FNamedOnlineSession* Session = GetNamedSessionFromLobbyId(*LobbyNetId);
	if (Session)
	{
		EOS_Lobby_CopyLobbyDetailsHandleOptions Options = {};
		Options.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLE_API_LATEST;
		Options.LobbyId = LobbyId;
		Options.LocalUserId = GetProductUserIdFromSubsystem(0);

		EOS_HLobbyDetails LobbyDetailsHandle;

		EOS_EResult CopyLobbyDetailsResult = EOS_Lobby_CopyLobbyDetailsHandle(this->LobbyHandle, &Options, &LobbyDetailsHandle);
		if (CopyLobbyDetailsResult == EOS_EResult::EOS_Success)
		{
			TSharedRef<FLobbyDetailsEOSKit> LobbyDetails = MakeShared<FLobbyDetailsEOSKit>(LobbyDetailsHandle);

			EOS_LobbyDetails_Info* LobbyDetailsInfo = nullptr;
			EOS_LobbyDetails_CopyInfoOptions CopyOptions = { };
			CopyOptions.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;
			
			EOS_EResult CopyInfoResult = EOS_LobbyDetails_CopyInfo(LobbyDetails->LobbyDetailsHandle, &CopyOptions, &LobbyDetailsInfo);
			if (CopyInfoResult == EOS_EResult::EOS_Success)
			{
				CopyLobbyData(LobbyDetails, LobbyDetailsInfo, *Session, [this, SessionName = Session->SessionName, LobbyDetails](bool bWasSuccessful) {
					if (bWasSuccessful)
					{
						if (FNamedOnlineSession* Session = GetNamedSession(SessionName))
						{
							TriggerOnSessionSettingsUpdatedDelegates(SessionName, Session->SessionSettings);
						}
					}
				});

				EOS_LobbyDetails_Info_Release(LobbyDetailsInfo);
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("[FOnlineSessionEOSKit::OnLobbyUpdateReceived] EOS_LobbyDetails_CopyInfo not successful. Finished with EOS_EResult %s"), ANSI_TO_TCHAR(EOS_EResult_ToString(CopyInfoResult)));
			}
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("[FOnlineSessionEOSKit::OnLobbyUpdateReceived] EOS_Lobby_CopyLobbyDetailsHandle not successful. Finished with EOS_EResult %s"), ANSI_TO_TCHAR(EOS_EResult_ToString(CopyLobbyDetailsResult)));
		}
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("[FOnlineSessionEOSKit::OnLobbyUpdateReceived] Unable to retrieve session with LobbyId %s"), *LobbyNetId->ToString());
	}
}

void FOnlineSessionEOSKit::OnLobbyMemberUpdateReceived(const EOS_LobbyId& LobbyId, const EOS_ProductUserId& TargetUserId)
{
	const TSharedRef<const FUniqueNetIdEOSLobby> LobbyNetId = FUniqueNetIdEOSLobby::Create(UTF8_TO_TCHAR(LobbyId));

	FUniqueNetIdPtr ResolvedUniqueNetId = ResolveProductUserIdToUniqueNetId(TargetUserId);
	if (ResolvedUniqueNetId.IsValid())
	{
		this->UpdateOrAddLobbyMember(LobbyNetId, ResolvedUniqueNetId.ToSharedRef());
	}
}

void FOnlineSessionEOSKit::OnMemberStatusReceived(const EOS_LobbyId& LobbyId, const EOS_ProductUserId& TargetUserId, EOS_ELobbyMemberStatus CurrentStatus)
{
	const TSharedRef<const FUniqueNetIdEOSLobby> LobbyNetId = FUniqueNetIdEOSLobby::Create(UTF8_TO_TCHAR(LobbyId));
	FNamedOnlineSession* Session = this->GetNamedSessionFromLobbyId(*LobbyNetId);
	if (Session)
	{
		switch (CurrentStatus)
		{
		case EOS_ELobbyMemberStatus::EOS_LMS_JOINED:
			{
				FUniqueNetIdPtr ResolvedUniqueNetId = ResolveProductUserIdToUniqueNetId(TargetUserId);
				if (ResolvedUniqueNetId.IsValid())
				{
					this->UpdateOrAddLobbyMember(LobbyNetId, ResolvedUniqueNetId.ToSharedRef());
				}
			}
			break;
		case EOS_ELobbyMemberStatus::EOS_LMS_LEFT:
			{
				FUniqueNetIdPtr ResolvedUniqueNetId = ResolveProductUserIdToUniqueNetId(TargetUserId);
				if (ResolvedUniqueNetId.IsValid())
				{
					FNamedOnlineSession* LeftSession = this->GetNamedSessionFromLobbyId(*LobbyNetId);
					if (LeftSession)
					{
						this->RemoveOnlineSessionMember(LeftSession->SessionName, ResolvedUniqueNetId.ToSharedRef());

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
						this->TriggerOnSessionParticipantLeftDelegates(LeftSession->SessionName, *ResolvedUniqueNetId, EOnSessionParticipantLeftReason::Left);
#else
						this->TriggerOnSessionParticipantsChangeDelegates(LeftSession->SessionName, *ResolvedUniqueNetId, false);
#endif
					}
				}
			}
			break;
		case EOS_ELobbyMemberStatus::EOS_LMS_DISCONNECTED:
			// OSS Session will end
			break;
		case EOS_ELobbyMemberStatus::EOS_LMS_KICKED:
			{
				FUniqueNetIdPtr ResolvedUniqueNetId = ResolveProductUserIdToUniqueNetId(TargetUserId);
				if (ResolvedUniqueNetId.IsValid())
				{
					FNamedOnlineSession* KickedSession = this->GetNamedSessionFromLobbyId(*LobbyNetId);
					if (KickedSession)
					{
						this->RemoveOnlineSessionMember(KickedSession->SessionName, ResolvedUniqueNetId.ToSharedRef());

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
						this->TriggerOnSessionParticipantLeftDelegates(KickedSession->SessionName, *ResolvedUniqueNetId, EOnSessionParticipantLeftReason::Kicked);
#else
						this->TriggerOnSessionParticipantRemovedDelegates(KickedSession->SessionName, *ResolvedUniqueNetId);
#endif
					}
				}
			}
			break;
		case EOS_ELobbyMemberStatus::EOS_LMS_PROMOTED:
			{
				FUniqueNetIdPtr ResolvedUniqueNetId = ResolveProductUserIdToUniqueNetId(TargetUserId);
				if (ResolvedUniqueNetId.IsValid())
				{
					FNamedOnlineSession* PromotedSession = this->GetNamedSessionFromLobbyId(*LobbyNetId);
					if (PromotedSession)
					{
						int32 DefaultLocalUser = 0;
						FUniqueNetIdPtr LocalPlayerUniqueNetId = EOSKitSubsystem && EOSKitSubsystem->GetIdentityInterface() ? EOSKitSubsystem->GetIdentityInterface()->GetUniquePlayerId(DefaultLocalUser) : nullptr;

						if (LocalPlayerUniqueNetId.IsValid() && *LocalPlayerUniqueNetId == *ResolvedUniqueNetId)
						{
							PromotedSession->OwningUserId = LocalPlayerUniqueNetId;
							PromotedSession->OwningUserName = EOSKitSubsystem && EOSKitSubsystem->GetIdentityInterface() ? EOSKitSubsystem->GetIdentityInterface()->GetPlayerNickname(DefaultLocalUser) : TEXT("");
							PromotedSession->bHosting = true;

							this->UpdateLobbySession(PromotedSession);
						}
					}
				}
			}
			break;
		case EOS_ELobbyMemberStatus::EOS_LMS_CLOSED:
			// OSS Session will end
			break;
		}
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("[FOnlineSessionEOSKit::OnMemberStatusReceived] Unable to retrieve session with LobbyId %s"), *LobbyNetId->ToString());
	}
}

void FOnlineSessionEOSKit::OnLobbyInviteAccepted(const char* InviteId, const EOS_ProductUserId& LocalUserId, const EOS_ProductUserId& TargetUserId)
{
	FUniqueNetIdPtr NetId = ResolveProductUserIdToUniqueNetId(LocalUserId);
	if (!NetId.IsValid())
	{
		FString ProductUserIdStr;
		char ProductUserIdBuffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t BufferSize = sizeof(ProductUserIdBuffer);
		if (EOS_ProductUserId_ToString(LocalUserId, ProductUserIdBuffer, &BufferSize) == EOS_EResult::EOS_Success)
		{
			ProductUserIdStr = UTF8_TO_TCHAR(ProductUserIdBuffer);
		}
		UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::OnLobbyInviteAccepted] Cannot accept lobby invite due to unknown user (%s)"), *ProductUserIdStr);
		TriggerOnSessionUserInviteAcceptedDelegates(false, 0, NetId, FOnlineSessionSearchResult());
		return;
	}
	int32 LocalUserNum = 0; // TODO: Get from NetId

	EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions Options = { };
		Options.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLEBYINVITEID_API_LATEST;
	Options.InviteId = InviteId;

	EOS_HLobbyDetails LobbyDetailsHandle;

	EOS_EResult Result = EOS_Lobby_CopyLobbyDetailsHandleByInviteId(LobbyHandle, &Options, &LobbyDetailsHandle);
	if (Result == EOS_EResult::EOS_Success)
	{
		TSharedRef<FLobbyDetailsEOSKit> LobbyDetails = MakeShared<FLobbyDetailsEOSKit>(LobbyDetailsHandle);

		LastInviteSearch = MakeShared<FOnlineSessionSearch>();
		AddLobbySearchResult(LobbyDetails, LastInviteSearch.ToSharedRef(), [this, LocalUserNum, NetId](bool bWasSuccessful)
		{
			// If we fail to copy the lobby data, we won't add a new search result, so we'll return an empty one
			TriggerOnSessionUserInviteAcceptedDelegates(bWasSuccessful, LocalUserNum, NetId, bWasSuccessful ? LastInviteSearch->SearchResults.Last() : FOnlineSessionSearchResult());
		});
	}
	else
	{
		UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::OnLobbyInviteAccepted] EOS_Lobby_CopyLobbyDetailsHandleByInviteId failed with EOS result code (%s)"), ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
		TriggerOnSessionUserInviteAcceptedDelegates(false, LocalUserNum, NetId, FOnlineSessionSearchResult());
	}
}

void FOnlineSessionEOSKit::OnJoinLobbyAccepted(const EOS_ProductUserId& LocalUserId, const EOS_UI_EventId& UiEventId)
{
	FUniqueNetIdPtr NetId = ResolveProductUserIdToUniqueNetId(LocalUserId);
	if (!NetId.IsValid())
	{
		FString ProductUserIdStr;
		char ProductUserIdBuffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t BufferSize = sizeof(ProductUserIdBuffer);
		if (EOS_ProductUserId_ToString(LocalUserId, ProductUserIdBuffer, &BufferSize) == EOS_EResult::EOS_Success)
		{
			ProductUserIdStr = UTF8_TO_TCHAR(ProductUserIdBuffer);
		}
		UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::OnJoinLobbyAccepted] Cannot join lobby due to unknown user (%s)"), *ProductUserIdStr);
		TriggerOnSessionUserInviteAcceptedDelegates(false, 0, NetId, FOnlineSessionSearchResult());
		return;
	}
	int32 LocalUserNum = 0; // TODO: Get from NetId

	EOS_Lobby_CopyLobbyDetailsHandleByUiEventIdOptions Options = { 0 };
		Options.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLEBYUIEVENTID_API_LATEST;
	Options.UiEventId = UiEventId;

	EOS_HLobbyDetails LobbyDetailsHandle;
	EOS_EResult Result = EOS_Lobby_CopyLobbyDetailsHandleByUiEventId(LobbyHandle, &Options, &LobbyDetailsHandle);
	if (Result == EOS_EResult::EOS_Success)
	{
		TSharedRef<FLobbyDetailsEOSKit> LobbyDetails = MakeShared<FLobbyDetailsEOSKit>(LobbyDetailsHandle);

		LastInviteSearch = MakeShared<FOnlineSessionSearch>();
		AddLobbySearchResult(LobbyDetails, LastInviteSearch.ToSharedRef(), [this, LocalUserNum, NetId](bool bWasSuccessful)
		{
			// If we fail to copy the lobby data, we won't add a new search result, so we'll return an empty one
			TriggerOnSessionUserInviteAcceptedDelegates(bWasSuccessful, LocalUserNum, NetId, bWasSuccessful ? LastInviteSearch->SearchResults.Last() : FOnlineSessionSearchResult());
		});
	}
	else
	{
		UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::OnJoinLobbyAccepted] EOS_Lobby_CopyLobbyDetailsHandleByUiEventId failed with EOS result code (%s)"), ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
		TriggerOnSessionUserInviteAcceptedDelegates(false, LocalUserNum, NetId, FOnlineSessionSearchResult());
	}
}

#if PLATFORM_WINDOWS
void FOnlineSessionEOSKit::OnLeaveLobbyRequested(const EOS_ProductUserId& LocalUserId, const EOS_Lobby_LeaveLobbyRequestedCallbackInfo* Data)
{
	// For now, just leave the lobby
	EOS_Lobby_LeaveLobbyOptions LeaveLobbyOptionsObj;
		LeaveLobbyOptionsObj.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;

	LeaveLobbyOptionsObj.LobbyId = Data->LobbyId;
	LeaveLobbyOptionsObj.LocalUserId = LocalUserId;
	EOS_Lobby_LeaveLobby(LobbyHandle, &LeaveLobbyOptionsObj, nullptr, nullptr);
}
#endif

// Helper function to convert ProductUserId to string
FString FOnlineSessionEOSKit::ProductUserIdToString(EOS_ProductUserId ProductUserId) const
{
	char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
	if (EOS_ProductUserId_ToString(ProductUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
	{
		return UTF8_TO_TCHAR(ProductUserIdStr);
	}
	return FString();
}

bool FOnlineSessionEOSKit::IsSessionSettingTypeSupported(EOnlineKeyValuePairDataType::Type InType)
{
	switch (InType)
	{
		case EOnlineKeyValuePairDataType::Int32:
		case EOnlineKeyValuePairDataType::UInt32:
		case EOnlineKeyValuePairDataType::Int64:
		case EOnlineKeyValuePairDataType::Double:
		case EOnlineKeyValuePairDataType::String:
		case EOnlineKeyValuePairDataType::Float:
		case EOnlineKeyValuePairDataType::Bool:
		case EOnlineKeyValuePairDataType::Json:
		{
			return true;
		}
	}
	return false;
}

EOS_EOnlineComparisonOp FOnlineSessionEOSKit::ToEOSSearchOp(EOnlineComparisonOp::Type Op)
{
	switch (Op)
	{
		case EOnlineComparisonOp::Equals:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_EQUAL;
		}
		case EOnlineComparisonOp::NotEquals:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_NOTEQUAL;
		}
		case EOnlineComparisonOp::GreaterThan:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_GREATERTHAN;
		}
		case EOnlineComparisonOp::GreaterThanEquals:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_GREATERTHANOREQUAL;
		}
		case EOnlineComparisonOp::LessThan:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_LESSTHAN;
		}
		case EOnlineComparisonOp::LessThanEquals:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_LESSTHANOREQUAL;
		}
		case EOnlineComparisonOp::Near:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_DISTANCE;
		}
		case EOnlineComparisonOp::In:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_ANYOF;
		}
		case EOnlineComparisonOp::NotIn:
		{
			return EOS_EOnlineComparisonOp::EOS_OCO_NOTANYOF;
		}
	}
	return EOS_EOnlineComparisonOp::EOS_OCO_EQUAL;
}

EOS_ELobbyPermissionLevel FOnlineSessionEOSKit::GetLobbyPermissionLevelFromSessionSettings(const FOnlineSessionSettings& SessionSettings)
{
	EOS_ELobbyPermissionLevel Result;

	if (SessionSettings.NumPublicConnections > 0)
	{
		Result = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
	}
	else if (SessionSettings.bAllowJoinViaPresence)
	{
		Result = EOS_ELobbyPermissionLevel::EOS_LPL_JOINVIAPRESENCE;
	}
	else
	{
		Result = EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY;
	}

	return Result;
}

uint32_t FOnlineSessionEOSKit::GetLobbyMaxMembersFromSessionSettings(const FOnlineSessionSettings& SessionSettings)
{
	return SessionSettings.NumPrivateConnections + SessionSettings.NumPublicConnections;
}

uint32 FOnlineSessionEOSKit::CreateLobbySession(int32 HostingPlayerNum, FNamedOnlineSession* Session)
{
	check(Session != nullptr);
	Session->SessionState = EOnlineSessionState::Creating;
	Session->bHosting = true;

	const EOS_ProductUserId LocalProductUserId = GetProductUserIdFromSubsystem(HostingPlayerNum);
	const FUniqueNetIdPtr LocalUserNetId = EOSKitSubsystem && EOSKitSubsystem->GetIdentityInterface() ? EOSKitSubsystem->GetIdentityInterface()->GetUniquePlayerId(HostingPlayerNum) : nullptr;
	
	if (!LocalProductUserId)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("[FOnlineSessionEOSKit::CreateLobbySession] Invalid ProductUserId"));
		Session->SessionState = EOnlineSessionState::NoSession;
		RemoveNamedSession(Session->SessionName);
		TriggerOnCreateSessionCompleteDelegates(Session->SessionName, false);
		return ONLINE_FAIL;
	}

	bool bUseHostMigration = true;
	FString HostMigrationSetting;
	if (Session->SessionSettings.Get(TEXT("SETTING_HOST_MIGRATION"), HostMigrationSetting))
	{
		bUseHostMigration = HostMigrationSetting == TEXT("true") || HostMigrationSetting == TEXT("1");
	}

	EOS_Lobby_CreateLobbyOptions CreateLobbyOptions = { 0 };
	CreateLobbyOptions.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	CreateLobbyOptions.LocalUserId = LocalProductUserId;
	CreateLobbyOptions.MaxLobbyMembers = GetLobbyMaxMembersFromSessionSettings(Session->SessionSettings);
	CreateLobbyOptions.PermissionLevel = GetLobbyPermissionLevelFromSessionSettings(Session->SessionSettings);
	CreateLobbyOptions.bPresenceEnabled = Session->SessionSettings.bUsesPresence;
	CreateLobbyOptions.bAllowInvites = Session->SessionSettings.bAllowInvites;
	CreateLobbyOptions.BucketId = BucketIdAnsi;
	CreateLobbyOptions.bDisableHostMigration = !bUseHostMigration;
#if WITH_EOS_RTC
	CreateLobbyOptions.bEnableRTCRoom = Session->SessionSettings.bUseLobbiesVoiceChatIfAvailable;
#endif

	FString SessionIdOverride;
	if (Session->SessionSettings.Get(TEXT("SETTING_SESSION_ID_OVERRIDE"), SessionIdOverride))
	{
		const FTCHARToUTF8 Utf8SessionIdOverride(*SessionIdOverride);
		if (SessionIdOverride.Len() >= EOS_LOBBY_MIN_LOBBYIDOVERRIDE_LENGTH && SessionIdOverride.Len() <= EOS_LOBBY_MAX_LOBBYIDOVERRIDE_LENGTH)
		{
			CreateLobbyOptions.LobbyId = Utf8SessionIdOverride.Get();
		}
		else if (!SessionIdOverride.IsEmpty())
		{
			UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::CreateLobbySession] Session setting SessionIdOverride is of invalid length [%d]. Valid length range is between %d and %d."), SessionIdOverride.Len(), EOS_LOBBY_MIN_LOBBYIDOVERRIDE_LENGTH, EOS_LOBBY_MAX_LOBBYIDOVERRIDE_LENGTH);
		}
	}

	FName SessionName = Session->SessionName;
#if ENGINE_MAJOR_VERSION == 5
	FLobbyCreatedCallback* CallbackObj = new FLobbyCreatedCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
#else
	FLobbyCreatedCallback* CallbackObj = new FLobbyCreatedCallback();
#endif
	LobbyCreatedCallback = static_cast<void*>(CallbackObj);
	CallbackObj->CallbackLambda = [this, SessionName, LocalProductUserId, LocalUserNetId](const EOS_Lobby_CreateLobbyCallbackInfo* Data)
	{
		FNamedOnlineSession* Session = GetNamedSession(SessionName);
		if (Session)
		{
			bool bWasSuccessful = Data->ResultCode == EOS_EResult::EOS_Success;
			if (bWasSuccessful)
			{
				UE_LOG_ONLINE_SESSION(Verbose, TEXT("[FOnlineSessionEOSKit::CreateLobbySession] CreateLobby was successful. LobbyId is %hs."), Data->LobbyId);

				Session->SessionState = EOnlineSessionState::Pending;

				// Because some platforms remap ports, we will use the ID of the name of the net driver to be our port instead
				FName NetDriverName = NAME_GameNetDriver;
				FString ProductUserIdStr = ProductUserIdToString(LocalProductUserId);
				FString HostAddr = FString::Printf(TEXT("EOS:%s:%s:26"), *ProductUserIdStr, *NetDriverName.ToString());

				TSharedPtr<FOnlineSessionInfoEOSKit> SessionInfo = MakeShareable(new FOnlineSessionInfoEOSKit(HostAddr));
				FString LobbyIdStr = UTF8_TO_TCHAR(Data->LobbyId);
				SessionInfo->SetSessionId(LobbyIdStr);
				Session->SessionInfo = SessionInfo;

				UpdateLobbySession(Session);
			}
			else
			{
				UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::CreateLobbySession] CreateLobby not successful. Finished with EOS_EResult %s"), ANSI_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));

				Session->SessionState = EOnlineSessionState::NoSession;

				RemoveNamedSession(SessionName);
			}

			TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);
		}
	};

	EOS_Lobby_CreateLobby(LobbyHandle, &CreateLobbyOptions, CallbackObj, CallbackObj->GetCallbackPtr());

	return ONLINE_IO_PENDING;
}

uint32 FOnlineSessionEOSKit::FindLobbySession(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	uint32 Result = ONLINE_FAIL;

	if (!LobbyHandle)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("[FOnlineSessionEOSKit::FindLobbySession] LobbyHandle is null"));
		return ONLINE_FAIL;
	}

	EOS_Lobby_CreateLobbySearchOptions CreateLobbySearchOptions = { 0 };
	CreateLobbySearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
	CreateLobbySearchOptions.MaxResults = FMath::Clamp(SearchSettings->MaxSearchResults, 0, EOS_SESSIONS_MAX_SEARCH_RESULTS);

	EOS_HLobbySearch LobbySearchHandle;

	EOS_EResult SearchResult = EOS_Lobby_CreateLobbySearch(LobbyHandle, &CreateLobbySearchOptions, &LobbySearchHandle);
	if (SearchResult == EOS_EResult::EOS_Success)
	{
		FString SessionSearchByUser;
		if(!SearchSettings->QuerySettings.Get(TEXT("SessionSearchByUser"), SessionSearchByUser))
		{
		// We add the search parameters (skip SEARCH_LOBBIES as it's a routing flag, not a search attribute)
		for (FSearchParams::TConstIterator It(SearchSettings->QuerySettings.SearchParams); It; ++It)
		{
			const FName Key = It.Key();
			
			// Skip SEARCH_LOBBIES - it's used for routing, not as a search parameter
			if (Key == SEARCH_LOBBIES)
			{
				continue;
			}
			
			const FOnlineSessionSearchParam& SearchParam = It.Value();
			if (!IsSessionSettingTypeSupported(SearchParam.Data.GetType()))
			{
				continue;
			}
			
			UE_LOG_ONLINE_SESSION(VeryVerbose, TEXT("[FOnlineSessionEOSKit::FindLobbySession] Adding lobby search param named (%s), (%s)"), *Key.ToString(), *SearchParam.ToString());

			FString ParamName(Key.ToString());
			FLobbyAttributeOptions Attribute(TCHAR_TO_UTF8(*ParamName), SearchParam.Data);
			AddLobbySearchAttribute(LobbySearchHandle, &Attribute, ToEOSSearchOp(SearchParam.ComparisonOp));
		}
		}
		else
		{
			EOS_LobbySearch_SetTargetUserIdOptions SetTargetUserIdOptions;
			SetTargetUserIdOptions.ApiVersion = EOS_LOBBYSEARCH_SETTARGETUSERID_API_LATEST;
			EOS_ProductUserId TargetUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*SessionSearchByUser));
			SetTargetUserIdOptions.TargetUserId = TargetUserId;
			EOS_LobbySearch_SetTargetUserId(LobbySearchHandle, &SetTargetUserIdOptions);
		}

		StartLobbySearch(SearchingPlayerNum, LobbySearchHandle, SearchSettings, FOnSingleSessionResultCompleteDelegate::CreateLambda([this](int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& EOSResult)
		{
			TriggerOnFindSessionsCompleteDelegates(bWasSuccessful);
		}));

		Result = ONLINE_IO_PENDING;
	}
	else
	{
		UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::FindLobbySession] CreateLobbySearch not successful. Finished with EOS_EResult %s"), ANSI_TO_TCHAR(EOS_EResult_ToString(SearchResult)));
	}

	return Result;
}

void FOnlineSessionEOSKit::StartLobbySearch(int32 SearchingPlayerNum, EOS_HLobbySearch LobbySearchHandle, const TSharedRef<FOnlineSessionSearch>& SearchSettings, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate)
{
	// When starting a new search, we'll reset the cache
	LobbySearchResultsCache.Reset();

	SessionSearchStartInSeconds = FPlatformTime::Seconds();

	EOS_LobbySearch_FindOptions FindOptions = { 0 };
	FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
	FindOptions.LocalUserId = GetProductUserIdFromSubsystem(SearchingPlayerNum);

	if (!FindOptions.LocalUserId)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("[FOnlineSessionEOSKit::StartLobbySearch] Invalid LocalUserId"));
		CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::Failed;
		CompletionDelegate.ExecuteIfBound(SearchingPlayerNum, false, FOnlineSessionSearchResult());
		return;
	}

#if ENGINE_MAJOR_VERSION == 5
	FLobbySearchFindCallback* CallbackObj = new FLobbySearchFindCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
#else
	FLobbySearchFindCallback* CallbackObj = new FLobbySearchFindCallback();
#endif
	LobbySearchFindCallback = static_cast<void*>(CallbackObj);
	CallbackObj->CallbackLambda = [this, SearchingPlayerNum, LobbySearchHandle, SearchSettings, CompletionDelegate](const EOS_LobbySearch_FindCallbackInfo* Data)
	{
		if (Data->ResultCode == EOS_EResult::EOS_Success)
		{
			UE_LOG_ONLINE_SESSION(Log, TEXT("[FOnlineSessionEOSKit::StartLobbySearch] LobbySearch_Find was successful."));

			if (CurrentSessionSearch.IsValid())
			{
				CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::Done;
			}

			EOS_LobbySearch_GetSearchResultCountOptions GetSearchResultCountOptions = { 0 };
			GetSearchResultCountOptions.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;

			uint32_t SearchResultsCount = EOS_LobbySearch_GetSearchResultCount(LobbySearchHandle, &GetSearchResultCountOptions);

			if (SearchResultsCount > 0)
			{
				EOS_LobbySearch_CopySearchResultByIndexOptions CopySearchResultByIndexOptions = { 0 };
				CopySearchResultByIndexOptions.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;

				for (uint32_t LobbyIndex = 0; LobbyIndex < SearchResultsCount; LobbyIndex++)
				{
					EOS_HLobbyDetails LobbyDetailsHandle;

					CopySearchResultByIndexOptions.LobbyIndex = LobbyIndex;

					EOS_EResult Result = EOS_LobbySearch_CopySearchResultByIndex(LobbySearchHandle, &CopySearchResultByIndexOptions, &LobbyDetailsHandle);
					if (Result == EOS_EResult::EOS_Success)
					{
						UE_LOG_ONLINE_SESSION(Verbose, TEXT("[FOnlineSessionEOSKit::StartLobbySearch::FLobbySearchFindCallback] LobbySearch_CopySearchResultByIndex was successful."));
						const TSharedRef<FLobbyDetailsEOSKit> LobbyDetails = MakeShared<FLobbyDetailsEOSKit>(LobbyDetailsHandle);
						PendingLobbySearchResults.Add(LobbyDetails);
					}
					else
					{
						UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::StartLobbySearch::FLobbySearchFindCallback] LobbySearch_CopySearchResultByIndex not successful. Finished with EOS_EResult %s"), ANSI_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
					}
				}
				
				// Make a copy to iterate over, as the AddLobbySearchResult delegate removes entries.
				const TArray<TSharedRef<FLobbyDetailsEOSKit>> LobbySearchResultsPendingIdResolutionCopy = PendingLobbySearchResults;
				for (const TSharedRef<FLobbyDetailsEOSKit>& LobbyDetails : LobbySearchResultsPendingIdResolutionCopy)
				{
					AddLobbySearchResult(LobbyDetails, SearchSettings, [this, LobbyDetails, CompletionDelegate, SearchingPlayerNum, SearchSettings](bool bWasSuccessful)
					{
						PendingLobbySearchResults.Remove(LobbyDetails);

						if (PendingLobbySearchResults.Num() < 1)
						{
							// If we fail to copy the lobby data, we won't add a new search result, so we'll return an empty one
							CompletionDelegate.ExecuteIfBound(SearchingPlayerNum, bWasSuccessful, bWasSuccessful ? SearchSettings->SearchResults.Last() : FOnlineSessionSearchResult());
						}
					});
				}

				if (PendingLobbySearchResults.Num() == 0)
				{
					CompletionDelegate.ExecuteIfBound(SearchingPlayerNum, true, SearchSettings->SearchResults.Last());
				}
			}
			else
			{
				UE_LOG_ONLINE_SESSION(Log, TEXT("[FOnlineSessionEOSKit::StartLobbySearch::FLobbySearchFindCallback] LobbySearch_GetSearchResultCount returned no results"));

				CompletionDelegate.ExecuteIfBound(SearchingPlayerNum, true, FOnlineSessionSearchResult());
			}
		}
		else
		{
			UE_LOG_ONLINE_SESSION(Warning, TEXT("[FOnlineSessionEOSKit::StartLobbySearch::FLobbySearchFindCallback] LobbySearch_Find not successful. Finished with EOS_EResult %s"), ANSI_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));

			if (CurrentSessionSearch.IsValid())
			{
				CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::Failed;
			}

			CompletionDelegate.ExecuteIfBound(SearchingPlayerNum, false, FOnlineSessionSearchResult());
		}

		EOS_LobbySearch_Release(LobbySearchHandle);
	};

	EOS_LobbySearch_Find(LobbySearchHandle, &FindOptions, CallbackObj, CallbackObj->GetCallbackPtr());
}

void FOnlineSessionEOSKit::FindEOSSessionById(int32 LocalUserNum, const FUniqueNetId& SessionId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate)
{
	if (!SessionsHandle)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("FindEOSSessionById: SessionsHandle is null"));
		CompletionDelegate.ExecuteIfBound(LocalUserNum, false, FOnlineSessionSearchResult());
		return;
	}

	EOS_ProductUserId LocalProductUserId = GetProductUserIdFromSubsystem(LocalUserNum);
	if (!LocalProductUserId)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("FindEOSSessionById: Invalid ProductUserId for LocalUserNum %d"), LocalUserNum);
		CompletionDelegate.ExecuteIfBound(LocalUserNum, false, FOnlineSessionSearchResult());
		return;
	}

	// Create session search
	EOS_HSessionSearch SearchHandle = nullptr;
	EOS_Sessions_CreateSessionSearchOptions HandleOptions = {};
	HandleOptions.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
	HandleOptions.MaxSearchResults = 1;

	EOS_EResult ResultCode = EOS_Sessions_CreateSessionSearch(SessionsHandle, &HandleOptions, &SearchHandle);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("FindEOSSessionById: EOS_Sessions_CreateSessionSearch failed: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
		CompletionDelegate.ExecuteIfBound(LocalUserNum, false, FOnlineSessionSearchResult());
		return;
	}

	// Set session ID to search for
	FString SessionIdStr = SessionId.ToString();
	EOS_SessionSearch_SetSessionIdOptions SetSessionIdOptions = {};
	SetSessionIdOptions.ApiVersion = EOS_SESSIONSEARCH_SETSESSIONID_API_LATEST;
	FTCHARToUTF8 SessionIdUtf8(*SessionIdStr);
	SetSessionIdOptions.SessionId = SessionIdUtf8.Get();

	EOS_EResult SetSessionIdResult = EOS_SessionSearch_SetSessionId(SearchHandle, &SetSessionIdOptions);
	if (SetSessionIdResult != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("FindEOSSessionById: EOS_SessionSearch_SetSessionId failed: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(SetSessionIdResult)));
		EOS_SessionSearch_Release(SearchHandle);
		CompletionDelegate.ExecuteIfBound(LocalUserNum, false, FOnlineSessionSearchResult());
		return;
	}

	// Store search handle
	TSharedPtr<FSessionSearchHandleWrapper> SharedSearchHandle = MakeShareable(new FSessionSearchHandleWrapper(SearchHandle));

	FFindSessionsCallback* CallbackObj = new FFindSessionsCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
	CallbackObj->CallbackLambda = [this, LocalUserNum, SharedSearchHandle, CompletionDelegate](const EOS_SessionSearch_FindCallbackInfo* Data)
	{
		bool bWasSuccessful = (Data->ResultCode == EOS_EResult::EOS_Success);
		
		if (bWasSuccessful)
		{
			EOS_SessionSearch_GetSearchResultCountOptions CountOptions = {};
			CountOptions.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
			int32 NumResults = EOS_SessionSearch_GetSearchResultCount(SharedSearchHandle->SearchHandle, &CountOptions);
			
			if (NumResults > 0)
			{
				EOS_SessionSearch_CopySearchResultByIndexOptions IndexOptions = {};
				IndexOptions.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
				IndexOptions.SessionIndex = 0;
				
				EOS_HSessionDetails SessionDetailsHandle = nullptr;
				EOS_EResult CopyResult = EOS_SessionSearch_CopySearchResultByIndex(SharedSearchHandle->SearchHandle, &IndexOptions, &SessionDetailsHandle);
				if (CopyResult == EOS_EResult::EOS_Success && SessionDetailsHandle)
				{
					FOnlineSessionSearchResult Result;
					CopySearchResult(SessionDetailsHandle, nullptr, Result.Session);
					CompletionDelegate.ExecuteIfBound(LocalUserNum, true, Result);
					return;
				}
			}
		}
		
		CompletionDelegate.ExecuteIfBound(LocalUserNum, false, FOnlineSessionSearchResult());
	};

	EOS_SessionSearch_FindOptions FindOptions = {};
	FindOptions.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
	FindOptions.LocalUserId = LocalProductUserId;
	
	EOS_SessionSearch_Find(SearchHandle, &FindOptions, CallbackObj, CallbackObj->GetCallbackPtr());
}

uint32 FOnlineSessionEOSKit::JoinEOSSession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession)
{
	if (!Session->SessionInfo.IsValid())
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("Session (%s) has invalid session info"), *Session->SessionName.ToString());
		return ONLINE_FAIL;
	}
	
	EOS_ProductUserId ProductUserId = GetProductUserIdFromSubsystem(PlayerNum);
	if (!EOS_ProductUserId_IsValid(ProductUserId))
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("Session (%s) invalid user id (%d)"), *Session->SessionName.ToString(), PlayerNum);
		return ONLINE_FAIL;
	}
	
	TSharedPtr<FOnlineSessionInfoEOSKit> EOSSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoEOSKit>(Session->SessionInfo);
	if (!EOSSessionInfo->SessionIdUnique.IsValid() || !EOSSessionInfo->SessionIdUnique->IsValid())
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("Session (%s) has invalid session id"), *Session->SessionName.ToString());
		return ONLINE_FAIL;
	}

	// Copy the session info over from search result
	TSharedPtr<const FOnlineSessionInfoEOSKit> SearchSessionInfo = StaticCastSharedPtr<const FOnlineSessionInfoEOSKit>(SearchSession->SessionInfo);
	if (SearchSessionInfo.IsValid())
	{
		// Copy EOSAddress and HostAddr from search result
		EOSSessionInfo->EOSAddress = SearchSessionInfo->EOSAddress;
		EOSSessionInfo->HostAddr = SearchSessionInfo->HostAddr;
		
		// CRITICAL: Copy the SessionHandle from search result (needed for joining)
		// EOS handles are reference-counted, so we can use the same handle
		EOSSessionInfo->SessionHandle = SearchSessionInfo->SessionHandle;
	}
	else
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("Session (%s) has invalid search session info"), *Session->SessionName.ToString());
		return ONLINE_FAIL;
	}

	Session->SessionState = EOnlineSessionState::Pending;

	FName SessionName = Session->SessionName;

#if ENGINE_MAJOR_VERSION == 5
	FJoinSessionCallback* CallbackObj = new FJoinSessionCallback(FOnlineSessionEOSKitWeakPtr(AsShared()));
#else
	FJoinSessionCallback* CallbackObj = new FJoinSessionCallback();
#endif
	CallbackObj->CallbackLambda = [this, SessionName](const EOS_Sessions_JoinSessionCallbackInfo* Data)
	{
		bool bWasSuccessful = false;

		FNamedOnlineSession* Session = GetNamedSession(SessionName);
		if (Session)
		{
			bWasSuccessful = Data->ResultCode == EOS_EResult::EOS_Success;
			if (bWasSuccessful)
			{
				BeginSessionAnalytics(Session);
			}
			else
			{
				UE_LOG_ONLINE_SESSION(Error, TEXT("EOS_Sessions_JoinSession() failed for session (%s) with EOS result code (%s)"), *SessionName.ToString(), ANSI_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));

				Session->SessionState = EOnlineSessionState::NoSession;

				RemoveNamedSession(SessionName);
			}
		}

		TriggerOnJoinSessionCompleteDelegates(SessionName, bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
	};

	FJoinSessionOptions Options(TCHAR_TO_UTF8(*Session->SessionName.ToString()));
	Options.LocalUserId = ProductUserId;
	Options.SessionHandle = EOSSessionInfo->SessionHandle;
	
	if (!Options.SessionHandle)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("Session (%s) has invalid SessionHandle"), *Session->SessionName.ToString());
		return ONLINE_FAIL;
	}
	
	EOS_Sessions_JoinSession(SessionsHandle, &Options, CallbackObj, CallbackObj->GetCallbackPtr());

	return ONLINE_IO_PENDING;
}

uint32 FOnlineSessionEOSKit::CreateLANSession(int32 HostingPlayerNum, FNamedOnlineSession* Session)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::CreateLANSession: Not implemented"));
	return ONLINE_FAIL;
}

uint32 FOnlineSessionEOSKit::JoinLANSession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::JoinLANSession: Not implemented"));
	return ONLINE_FAIL;
}

uint32 FOnlineSessionEOSKit::JoinLobbySession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::JoinLobbySession: Not implemented"));
	return ONLINE_FAIL;
}

uint32 FOnlineSessionEOSKit::UpdateLobbySession(FNamedOnlineSession* Session)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::UpdateLobbySession: Not implemented"));
	return ONLINE_FAIL;
}

uint32 FOnlineSessionEOSKit::StartLobbySession(FNamedOnlineSession* Session)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::StartLobbySession: Not implemented"));
	return ONLINE_FAIL;
}

bool FOnlineSessionEOSKit::AddOnlineSessionMember(FName SessionName, const FUniqueNetIdRef& PlayerId)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::AddOnlineSessionMember: Not implemented"));
	return false;
}

bool FOnlineSessionEOSKit::RemoveOnlineSessionMember(FName SessionName, const FUniqueNetIdRef& PlayerId)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::RemoveOnlineSessionMember: Not implemented"));
	return false;
}

void FOnlineSessionEOSKit::CopyLobbyData(const TSharedRef<FLobbyDetailsEOSKit>& LobbyDetails, EOS_LobbyDetails_Info* LobbyDetailsInfo, FOnlineSession& OutSession, const FOnCopyLobbyDataCompleteCallback& Callback)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::CopyLobbyData: Not implemented"));
	if (Callback)
	{
		Callback(false);
	}
}

void FOnlineSessionEOSKit::CopyLobbyMemberAttributes(const FLobbyDetailsEOSKit& LobbyDetails, const EOS_ProductUserId& TargetUserId, FSessionSettings& OutSessionSettings)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::CopyLobbyMemberAttributes: Not implemented"));
}

void FOnlineSessionEOSKit::AddLobbySearchResult(const TSharedRef<FLobbyDetailsEOSKit>& LobbyDetails, const TSharedRef<FOnlineSessionSearch>& SearchSettings, const FOnCopyLobbyDataCompleteCallback& Callback)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::AddLobbySearchResult: Not implemented"));
	if (Callback)
	{
		Callback(false);
	}
}

void FOnlineSessionEOSKit::AddLobbySearchAttribute(EOS_HLobbySearch LobbySearchHandle, const EOS_Lobby_AttributeData* Attribute, EOS_EOnlineComparisonOp ComparisonOp)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::AddLobbySearchAttribute: Not implemented"));
}

void FOnlineSessionEOSKit::RegisterLobbyNotifications()
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::RegisterLobbyNotifications: Not implemented"));
}

void FOnlineSessionEOSKit::AddSearchResult(EOS_HSessionDetails SessionHandle, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	if (!SessionHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::AddSearchResult: Invalid SessionHandle"));
		return;
	}

	// Get session info
	EOS_SessionDetails_Info* SessionInfo = nullptr;
	EOS_SessionDetails_CopyInfoOptions InfoOptions = {};
	InfoOptions.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;
	
	if (EOS_SessionDetails_CopyInfo(SessionHandle, &InfoOptions, &SessionInfo) != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::AddSearchResult: Failed to copy session info"));
		return;
	}

	// Add to search results
	int32 ResultIndex = SearchSettings->SearchResults.AddZeroed();
	FOnlineSessionSearchResult& Result = SearchSettings->SearchResults[ResultIndex];
	
	// Copy session data
	CopySearchResult(SessionHandle, SessionInfo, Result.Session);
	
	EOS_SessionDetails_Info_Release(SessionInfo);
}

void FOnlineSessionEOSKit::CopySearchResult(EOS_HSessionDetails SessionHandle, EOS_SessionDetails_Info* SessionInfo, FOnlineSession& OutSession)
{
	if (!SessionHandle || !SessionInfo)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::CopySearchResult: Invalid parameters"));
		return;
	}

	// Create session info with SessionHandle (CRITICAL for joining!)
	FString SessionIdStr = UTF8_TO_TCHAR(SessionInfo->SessionId);
	FString HostAddrStr = UTF8_TO_TCHAR(SessionInfo->HostAddress);
	
	FOnlineSessionInfoEOSKit* NewSessionInfo = new FOnlineSessionInfoEOSKit(HostAddrStr);
	NewSessionInfo->SetSessionId(SessionIdStr);
	NewSessionInfo->SessionHandle = SessionHandle; // Store handle for joining!
	
	// CRITICAL: Set EOSAddress for P2P connections
	// Prefer HostAddress from EOS if it's already in EOS format (EOS:ProductUserId:...)
	// Otherwise, build it from OwnerUserId
	if (HostAddrStr.StartsWith(TEXT("EOS:"), ESearchCase::IgnoreCase))
	{
		// HostAddress is already in EOS format - use it directly
		NewSessionInfo->EOSAddress = HostAddrStr;
		UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::CopySearchResult: Using HostAddress as EOSAddress: %s"), *NewSessionInfo->EOSAddress);
	}
	else if (EOS_ProductUserId_IsValid(SessionInfo->OwnerUserId))
	{
		// Build EOS P2P address from OwnerUserId: EOS:HostProductUserId:GameNetDriver:26 (EIK format)
		char OwnerIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 BufferSize = sizeof(OwnerIdStr);
		if (EOS_ProductUserId_ToString(SessionInfo->OwnerUserId, OwnerIdStr, &BufferSize) == EOS_EResult::EOS_Success)
		{
			NewSessionInfo->EOSAddress = FString::Printf(TEXT("EOS:%s:GameNetDriver:26"), UTF8_TO_TCHAR(OwnerIdStr));
			UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::CopySearchResult: Built EOSAddress from OwnerUserId: %s"), *NewSessionInfo->EOSAddress);
		}
	}
	
	OutSession.SessionInfo = MakeShareable(NewSessionInfo);
	
	// Copy basic session data
	OutSession.NumOpenPublicConnections = SessionInfo->NumOpenPublicConnections;
	OutSession.NumOpenPrivateConnections = 0;
	OutSession.SessionSettings.NumPublicConnections = SessionInfo->Settings->NumPublicConnections;
	OutSession.SessionSettings.bShouldAdvertise = true;
	OutSession.SessionSettings.bAllowJoinInProgress = SessionInfo->Settings->bAllowJoinInProgress == EOS_TRUE;
	OutSession.SessionSettings.bIsLANMatch = false;
	OutSession.SessionSettings.bIsDedicated = false;
	OutSession.SessionSettings.bUseLobbiesIfAvailable = false; // Default to false for regular sessions
	
	// Convert OwnerUserId to string for display
	if (EOS_ProductUserId_IsValid(SessionInfo->OwnerUserId))
	{
		char OwnerIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 BufferSize = sizeof(OwnerIdStr);
		if (EOS_ProductUserId_ToString(SessionInfo->OwnerUserId, OwnerIdStr, &BufferSize) == EOS_EResult::EOS_Success)
		{
			OutSession.OwningUserName = UTF8_TO_TCHAR(OwnerIdStr);
		}
	}
	
	UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::CopySearchResult: Copied session - ID: %s, Host: %s, OpenSlots: %d/%d"), 
		*SessionIdStr, *HostAddrStr, SessionInfo->NumOpenPublicConnections, SessionInfo->Settings->NumPublicConnections);
	
	// CRITICAL: Copy all session attributes (including MapName) from EOS SessionDetails
	// This matches EIK's behavior - attributes like MapName are stored in session settings
	CopyAttributes(SessionHandle, OutSession);
}

void FOnlineSessionEOSKit::CopyAttributes(EOS_HSessionDetails SessionHandle, FOnlineSession& OutSession)
{
	if (!SessionHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSessionEOSKit::CopyAttributes: Invalid SessionHandle"));
		return;
	}
	
	// Get the number of session attributes
	EOS_SessionDetails_GetSessionAttributeCountOptions CountOptions = {};
	CountOptions.ApiVersion = EOS_SESSIONDETAILS_GETSESSIONATTRIBUTECOUNT_API_LATEST;
	int32 AttributeCount = EOS_SessionDetails_GetSessionAttributeCount(SessionHandle, &CountOptions);
	
	UE_LOG_ONLINE(Verbose, TEXT("FOnlineSessionEOSKit::CopyAttributes: Found %d session attributes"), AttributeCount);
	
	// Iterate through all attributes and copy them to SessionSettings
	for (int32 Index = 0; Index < AttributeCount; Index++)
	{
		EOS_SessionDetails_CopySessionAttributeByIndexOptions AttrOptions = {};
		AttrOptions.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST;
		AttrOptions.AttrIndex = Index;
		
		EOS_SessionDetails_Attribute* Attribute = nullptr;
		EOS_EResult ResultCode = EOS_SessionDetails_CopySessionAttributeByIndex(SessionHandle, &AttrOptions, &Attribute);
		
		if (ResultCode == EOS_EResult::EOS_Success && Attribute && Attribute->Data)
		{
			FString Key = UTF8_TO_TCHAR(Attribute->Data->Key);
			
			// Handle special attributes that map to FOnlineSession properties
			if (Key == TEXT("NumPublicConnections"))
			{
				OutSession.SessionSettings.NumPublicConnections = Attribute->Data->Value.AsInt64;
			}
			else if (Key == TEXT("NumPrivateConnections"))
			{
				OutSession.SessionSettings.NumPrivateConnections = Attribute->Data->Value.AsInt64;
			}
			else if (Key == TEXT("OwningUserId"))
			{
				// Store as OwningUserId if needed
				// OutSession.OwningUserId = ...;
			}
			else if (Key == TEXT("OwningUserName"))
			{
				OutSession.OwningUserName = UTF8_TO_TCHAR(Attribute->Data->Value.AsUtf8);
			}
			else if (Key == TEXT("bAntiCheatProtected"))
			{
				OutSession.SessionSettings.bAntiCheatProtected = Attribute->Data->Value.AsBool == EOS_TRUE;
			}
			else if (Key == TEXT("bUsesStats"))
			{
				OutSession.SessionSettings.bUsesStats = Attribute->Data->Value.AsBool == EOS_TRUE;
			}
			else if (Key == TEXT("bIsDedicated"))
			{
				OutSession.SessionSettings.bIsDedicated = Attribute->Data->Value.AsBool == EOS_TRUE;
			}
			else if (Key == TEXT("BuildUniqueId"))
			{
				OutSession.SessionSettings.BuildUniqueId = Attribute->Data->Value.AsInt64;
			}
			// Handle all other attributes (including MapName) as FOnlineSessionSetting
			else
			{
				FOnlineSessionSetting Setting;
				Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				
				switch (Attribute->Data->ValueType)
				{
					case EOS_ESessionAttributeType::EOS_SAT_Boolean:
					{
						Setting.Data.SetValue(Attribute->Data->Value.AsBool == EOS_TRUE);
						break;
					}
					case EOS_ESessionAttributeType::EOS_SAT_Int64:
					{
						Setting.Data.SetValue(int64(Attribute->Data->Value.AsInt64));
						break;
					}
					case EOS_ESessionAttributeType::EOS_SAT_Double:
					{
						Setting.Data.SetValue(Attribute->Data->Value.AsDouble);
						break;
					}
					case EOS_ESessionAttributeType::EOS_SAT_String:
					{
						Setting.Data.SetValue(UTF8_TO_TCHAR(Attribute->Data->Value.AsUtf8));
						break;
					}
				}
				
				OutSession.SessionSettings.Settings.Add(FName(Key), Setting);
				
				// Log MapName specifically for debugging
				if (Key == TEXT("MapName"))
				{
					FString MapNameValue;
					Setting.Data.GetValue(MapNameValue);
					UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::CopyAttributes: ✅ Copied MapName attribute: %s"), *MapNameValue);
				}
			}
		}
		
		if (Attribute)
		{
			EOS_SessionDetails_Attribute_Release(Attribute);
		}
	}
	
	UE_LOG_ONLINE(Verbose, TEXT("FOnlineSessionEOSKit::CopyAttributes: Copied %d attributes to SessionSettings"), AttributeCount);
}

void FOnlineSessionEOSKit::SetPermissionLevel(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session)
{
	EOS_SessionModification_SetPermissionLevelOptions Options = { };
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
	if (Session->SessionSettings.NumPublicConnections > 0)
	{
		Options.PermissionLevel = EOS_EOnlineSessionPermissionLevel::EOS_OSPF_PublicAdvertised;
	}
	else if (Session->SessionSettings.bAllowJoinViaPresence)
	{
		Options.PermissionLevel = EOS_EOnlineSessionPermissionLevel::EOS_OSPF_JoinViaPresence;
	}
	else
	{
		Options.PermissionLevel = EOS_EOnlineSessionPermissionLevel::EOS_OSPF_InviteOnly;
	}

	UE_LOG_ONLINE_SESSION(Log, TEXT("EOS_SessionModification_SetPermissionLevel() set to (%d) for session (%s)"), (int32)Options.PermissionLevel, *Session->SessionName.ToString());

	EOS_EResult ResultCode = EOS_SessionModification_SetPermissionLevel(SessionModHandle, &Options);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("EOS_SessionModification_SetPermissionLevel() failed with EOS result code (%s)"), ANSI_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
	}
}

void FOnlineSessionEOSKit::SetMaxPlayers(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session)
{
	EOS_SessionModification_SetMaxPlayersOptions Options = { };
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETMAXPLAYERS_API_LATEST;
	Options.MaxPlayers = Session->SessionSettings.NumPrivateConnections + Session->SessionSettings.NumPublicConnections;

	UE_LOG_ONLINE_SESSION(Log, TEXT("EOS_SessionModification_SetMaxPlayers() set to (%d) for session (%s)"), Options.MaxPlayers, *Session->SessionName.ToString());

	const EOS_EResult ResultCode = EOS_SessionModification_SetMaxPlayers(SessionModHandle, &Options);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("EOS_SessionModification_SetMaxPlayers() failed with EOS result code (%s)"), ANSI_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
	}
}

void FOnlineSessionEOSKit::SetInvitesAllowed(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session)
{
	EOS_SessionModification_SetInvitesAllowedOptions Options = { };
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETINVITESALLOWED_API_LATEST;
	Options.bInvitesAllowed = Session->SessionSettings.bAllowInvites ? EOS_TRUE : EOS_FALSE;

	UE_LOG_ONLINE_SESSION(Log, TEXT("EOS_SessionModification_SetInvitesAllowed() set to (%s) for session (%s)"), Options.bInvitesAllowed ? TEXT("true") : TEXT("false"), *Session->SessionName.ToString());

	const EOS_EResult ResultCode = EOS_SessionModification_SetInvitesAllowed(SessionModHandle, &Options);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("EOS_SessionModification_SetInvitesAllowed() failed with EOS result code (%s)"), ANSI_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
	}
}

void FOnlineSessionEOSKit::SetJoinInProgress(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session)
{
	EOS_SessionModification_SetJoinInProgressAllowedOptions Options = { };
	Options.ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST;
	Options.bAllowJoinInProgress = Session->SessionSettings.bAllowJoinInProgress ? EOS_TRUE : EOS_FALSE;

	UE_LOG_ONLINE_SESSION(Log, TEXT("EOS_SessionModification_SetJoinInProgressAllowed() set to (%s) for session (%s)"), Options.bAllowJoinInProgress ? TEXT("true") : TEXT("false"), *Session->SessionName.ToString());

	EOS_EResult ResultCode = EOS_SessionModification_SetJoinInProgressAllowed(SessionModHandle, &Options);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("EOS_SessionModification_SetJoinInProgressAllowed() failed with EOS result code (%s)"), ANSI_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
	}
}

void FOnlineSessionEOSKit::AddAttribute(EOS_HSessionModification SessionModHandle, const EOS_Sessions_AttributeData* Attribute)
{
	EOS_SessionModification_AddAttributeOptions Options = { };
	Options.ApiVersion = EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST;
	Options.AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise;
	Options.SessionAttribute = Attribute;

	UE_LOG_ONLINE_SESSION(Log, TEXT("EOS_SessionModification_AddAttribute() named (%s)"), UTF8_TO_TCHAR(Attribute->Key));

	EOS_EResult ResultCode = EOS_SessionModification_AddAttribute(SessionModHandle, &Options);
	if (ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("EOS_SessionModification_AddAttribute() failed for attribute name (%s) with EOS result code (%s)"), UTF8_TO_TCHAR(Attribute->Key), ANSI_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
	}
}

void FOnlineSessionEOSKit::SetAttributes(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session)
{
	// The first will let us find it on session searches
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
	const FString SearchPresence("PRESENCESEARCH");
#else
	const FString SearchPresence(SEARCH_PRESENCE.ToString());
#endif
	const FAttributeOptions SearchPresenceAttribute(TCHAR_TO_UTF8(*SearchPresence), true);
	AddAttribute(SessionModHandle, &SearchPresenceAttribute);

	FAttributeOptions Opt1("NumPrivateConnections", Session->SessionSettings.NumPrivateConnections);
	AddAttribute(SessionModHandle, &Opt1);

	FAttributeOptions Opt2("NumPublicConnections", Session->SessionSettings.NumPublicConnections);
	AddAttribute(SessionModHandle, &Opt2);

	if (Session->OwningUserId.IsValid() && Session->OwningUserId->IsValid())
	{
		FAttributeOptions OwningUserId("OwningUserId", TCHAR_TO_UTF8(*Session->OwningUserId->ToString()));
		AddAttribute(SessionModHandle, &OwningUserId);
	}

	// Handle auto generation of dedicated server names
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

	FAttributeOptions OwningUserName("OwningUserName", TCHAR_TO_UTF8(*Session->OwningUserName));
	AddAttribute(SessionModHandle, &OwningUserName);

	FAttributeOptions Opt5("bAntiCheatProtected", Session->SessionSettings.bAntiCheatProtected);
	AddAttribute(SessionModHandle, &Opt5);

	FAttributeOptions Opt6("bUsesStats", Session->SessionSettings.bUsesStats);
	AddAttribute(SessionModHandle, &Opt6);

	FAttributeOptions Opt7("bIsDedicated", Session->SessionSettings.bIsDedicated);
	AddAttribute(SessionModHandle, &Opt7);

	FAttributeOptions Opt8("BuildUniqueId", Session->SessionSettings.BuildUniqueId);
	AddAttribute(SessionModHandle, &Opt8);

	// Add all of the session settings
	for (FSessionSettings::TConstIterator It(Session->SessionSettings.Settings); It; ++It)
	{
		const FName KeyName = It.Key();
		const FOnlineSessionSetting& Setting = It.Value();

		// Skip unsupported types or non session advertised settings
		if (Setting.AdvertisementType < EOnlineDataAdvertisementType::ViaOnlineService || !IsSessionSettingTypeSupported(Setting.Data.GetType()))
		{
			continue;
		}

		FAttributeOptions Attribute(TCHAR_TO_UTF8(*KeyName.ToString()), Setting.Data);
		AddAttribute(SessionModHandle, &Attribute);
	}
}

uint32 FOnlineSessionEOSKit::SharedSessionUpdate(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session, void* InCallback)
{
	// Cast callback back to FUpdateSessionCallback*
	FUpdateSessionCallback* Callback = static_cast<FUpdateSessionCallback*>(InCallback);
	if (!Callback)
	{
		UE_LOG_ONLINE_SESSION(Error, TEXT("FOnlineSessionEOSKit::SharedSessionUpdate: Invalid callback"));
		return ONLINE_FAIL;
	}

	// Set joinability flags
	SetPermissionLevel(SessionModHandle, Session);
	// Set max players
	SetMaxPlayers(SessionModHandle, Session);
	// Set invite flags
	SetInvitesAllowed(SessionModHandle, Session);
	// Set JIP flag
	SetJoinInProgress(SessionModHandle, Session);
	// Add any attributes for filtering by searchers
	SetAttributes(SessionModHandle, Session);

	// Commit the session changes
	EOS_Sessions_UpdateSessionOptions CreateOptions = { };
	CreateOptions.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
	CreateOptions.SessionModificationHandle = SessionModHandle;
	EOS_Sessions_UpdateSession(SessionsHandle, &CreateOptions, Callback, Callback->GetCallbackPtr());

	EOS_SessionModification_Release(SessionModHandle);

	return ONLINE_IO_PENDING;
}

void FOnlineSessionEOSKit::BeginSessionAnalytics(FNamedOnlineSession* Session)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionEOSKit::BeginSessionAnalytics: Not implemented"));
}

#endif // WITH_EOS_SDK
