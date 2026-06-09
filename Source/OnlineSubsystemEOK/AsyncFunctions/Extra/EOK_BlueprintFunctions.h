// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "VoiceChat.h"
#include "eos_lobby_types.h"
#include "eos_lobby.h"

#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OnlineSubsystemEOK/AsyncFunctions/Sessions/EOK_UpdateSession_AsyncFunction.h"
#include "EOK_BlueprintFunctions.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnResponseFromSanctions, bool, bWasSuccess);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnResponseFromEpicForAccessToken, bool, bWasSuccess, const FString&, AccessToken);

UENUM(BlueprintType)
enum EEOSSanctionType
{
	IncorrectSanction,
	CompromisedAccount,
	UnfairPunishment,
	AppealForForgiveness,
};

UENUM(BlueprintType)
enum EEOK_LoginStatus
{
	/** Player has not logged in or chosen a local profile */
	NotLoggedIn,
	/** Player is using a local profile but is not logged in */
	UsingLocalProfile,
	/** Player has been validated by the platform specific authentication service */
	LoggedIn,
};

UENUM(BlueprintType)
enum ESessionCurrentState
{
	/** An online session has not been created yet */
	NoSession,
	/** An online session is in the process of being created */
	Creating,
	/** Session has been created but the session hasn't started (pre match lobby) */
	Pending,
	/** Session has been asked to start (may take time due to communication with backend) */
	Starting,
	/** The current session has started. Sessions with join in progress disabled are no longer joinable */
	InProgress,
	/** The session is still valid, but the session is no longer being played (post match lobby) */
	Ending,
	/** The session is closed and any stats committed */
	Ended,
	/** The session is being destroyed */
	Destroying
};

USTRUCT(BlueprintType)
struct FEOK_OnlineSessionSettings
{
	GENERATED_BODY()

	/** The number of publicly available connections advertised */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	int32 NumPublicConnections;
	/** The number of connections that are private (invite/password) only */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	int32 NumPrivateConnections;
	/** Whether this match is publicly advertised on the online service */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bShouldAdvertise;
	/** Whether joining in progress is allowed or not */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bAllowJoinInProgress;
	/** This game will be lan only and not be visible to external players */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bIsLANMatch;
	/** Whether the server is dedicated or player hosted */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bIsDedicated;
	/** Whether the match should gather stats or not */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bUsesStats;
	/** Whether the match allows invitations for this session or not */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bAllowInvites;
	/** Whether to display user presence information or not */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bUsesPresence;
	/** Whether joining via player presence is allowed or not */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bAllowJoinViaPresence;
	/** Whether joining via player presence is allowed for friends only or not */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bAllowJoinViaPresenceFriendsOnly;
	/** Whether the server employs anti-cheat (punkbuster, vac, etc) */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bAntiCheatProtected;
	/** Whether to prefer lobbies APIs if the platform supports them */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bUseLobbiesIfAvailable;
	/** Whether to create (and auto join) a voice chat room for the lobby, if the platform supports it */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bUseLobbiesVoiceChatIfAvailable;
	/** Manual override for the Session Id instead of having one assigned by the backend. Its size may be restricted depending on the platform */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	FString SessionIdOverride;

	/** Used to keep different builds from seeing each other during searches */
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	int32 BuildUniqueId;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	TMap<FString, FEOKAttribute> SessionSettings;
	
	FEOK_OnlineSessionSettings(FOnlineSessionSettings Settings)
	{
		NumPublicConnections = Settings.NumPublicConnections;
		NumPrivateConnections = Settings.NumPrivateConnections;
		bShouldAdvertise = Settings.bShouldAdvertise;
		bAllowJoinInProgress = Settings.bAllowJoinInProgress;
		bIsLANMatch = Settings.bIsLANMatch;
		bIsDedicated = Settings.bIsDedicated;
		bUsesStats = Settings.bUsesStats;
		bAllowInvites = Settings.bAllowInvites;
		bUsesPresence = Settings.bUsesPresence;
		bAllowJoinViaPresence = Settings.bAllowJoinViaPresence;
		bAllowJoinViaPresenceFriendsOnly = Settings.bAllowJoinViaPresenceFriendsOnly;
		bAntiCheatProtected = Settings.bAntiCheatProtected;
		bUseLobbiesIfAvailable = Settings.bUseLobbiesIfAvailable;
		bUseLobbiesVoiceChatIfAvailable = Settings.bUseLobbiesVoiceChatIfAvailable;
#if ENGINE_MAJOR_VERSION == 5
		SessionIdOverride = Settings.SessionIdOverride;
#endif
		BuildUniqueId = Settings.BuildUniqueId;
		TMap<FName, FOnlineSessionSetting>::TIterator It(Settings.Settings);
		TMap<FString, FEOKAttribute> LocalArraySettings;
		while (It)
		{
			const FName& SettingName = It.Key();
			const FOnlineSessionSetting& Setting = It.Value();
			LocalArraySettings.Add(*SettingName.ToString(), Setting.Data);
			++It;
		}
		SessionSettings = LocalArraySettings;
	}

	FEOK_OnlineSessionSettings()
	{
		NumPublicConnections = 0;
		NumPrivateConnections = 0;
		bShouldAdvertise = false;
		bAllowJoinInProgress = false;
		bIsLANMatch = false;
		bIsDedicated = false;
		bUsesStats = false;
		bAllowInvites = false;
		bUsesPresence = false;
		bAllowJoinViaPresence = false;
		bAllowJoinViaPresenceFriendsOnly = false;
		bAntiCheatProtected = false;
		bUseLobbiesIfAvailable = false;
		bUseLobbiesVoiceChatIfAvailable = false;
		SessionIdOverride = "";
		BuildUniqueId = 0;
	}
};

USTRUCT(BlueprintType)
struct FEOK_CurrentSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bHostingSession;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bPublicJoinable;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bFriendJoinable;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bInviteOnly;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	bool bAllowInvites;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	TArray<FEOKUniqueNetId> RegisteredPlayers;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	TArray<FEOK_MemberSpecificAttribute> MemberSettings;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	FEOKUniqueNetId SessionOwner;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	TEnumAsByte<ESessionCurrentState> SessionState;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	int32 NumOpenPublicConnections;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	int32 MaxPublicConnections;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	int32 NumOpenPrivateConnections;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	int32 MaxPrivateConnections;
	
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	FString SessionIdString;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	FString CompleteDebugString;

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Sessions")
	FEOK_OnlineSessionSettings OriginalSessionSettings;
	
	FEOK_CurrentSessionInfo(): bAllowInvites(false), SessionState(), NumOpenPublicConnections(0),
	                           MaxPublicConnections(0),
	                           NumOpenPrivateConnections(0), MaxPrivateConnections(0), OriginalSessionSettings()
	{
		bHostingSession = false;
		bPublicJoinable = false;
		bFriendJoinable = false;
		bInviteOnly = false;
	}

	FEOK_CurrentSessionInfo(FNamedOnlineSession Session)
	{
		CompleteDebugString = Session.SessionInfo->ToDebugString();
		NumOpenPublicConnections = Session.NumOpenPublicConnections;
		NumOpenPrivateConnections = Session.NumOpenPrivateConnections;
		MaxPublicConnections = Session.SessionSettings.NumPublicConnections;
		MaxPrivateConnections = Session.SessionSettings.NumPrivateConnections;
		SessionIdString = Session.GetSessionIdStr();
		OriginalSessionSettings = Session.SessionSettings;
		bHostingSession = Session.bHosting;
		Session.GetJoinability(bPublicJoinable, bFriendJoinable, bInviteOnly, bAllowInvites);
		FEOKUniqueNetId Temp;
		Temp.SetUniqueNetId(Session.OwningUserId);
		SessionOwner = Temp;
		for (auto& Player : Session.RegisteredPlayers)
		{
			FEOKUniqueNetId Temp1;
			Temp1.SetUniqueNetId(Player);
			RegisteredPlayers.Add(Temp1);
		}
		for (auto& LocalMemberSettings : Session.SessionSettings.MemberSettings)
		{
			FEOK_MemberSpecificAttribute Temp2;
			FEOKUniqueNetId MemberId;
			MemberId.SetUniqueNetId(LocalMemberSettings.Key);
			Temp2.MemberId = MemberId;
			TMap<FString, FEOKAttribute> LocalAttributes;
			for (auto& Attribute : LocalMemberSettings.Value)
			{
				FEOKAttribute Temp3 = Attribute.Value.Data;
				LocalAttributes.Add(Attribute.Key.ToString(), Temp3);
			}
			Temp2.Attributes = LocalAttributes;
			MemberSettings.Add(Temp2);
		}
		
		switch (Session.SessionState)
		{
		case EOnlineSessionState::NoSession:
			SessionState = ESessionCurrentState::NoSession;
			break;
		case EOnlineSessionState::Creating:
			SessionState = ESessionCurrentState::Creating;
			break;
		case EOnlineSessionState::Pending:
			SessionState = ESessionCurrentState::Pending;
			break;
		case EOnlineSessionState::Starting:
			SessionState = ESessionCurrentState::Starting;
			break;
		case EOnlineSessionState::InProgress:
			SessionState = ESessionCurrentState::InProgress;
			break;
		case EOnlineSessionState::Ending:
			SessionState = ESessionCurrentState::Ending;
			break;
		case EOnlineSessionState::Ended:
			SessionState = ESessionCurrentState::Ended;
			break;
		case EOnlineSessionState::Destroying:
			SessionState = ESessionCurrentState::Destroying;
			break;
		}
	}
};

UCLASS()
class OnlineSubsystemEOK_API UEOK_BlueprintFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Extra", meta=( WorldContext = "Context" ))
	static FString GetEpicAccountId(UObject* Context);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Extra", meta=( WorldContext = "Context" ), DisplayName="Get EOK Session Info")
	static FEOK_CurrentSessionInfo GetCurrentSessionInfo(UObject* Context, bool& bIsSessionPresent, FName SessionName = "GameSession");

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Extra", meta=( WorldContext = "Context" ), DisplayName="Get EOK Session Names")
	static TArray<FName> GetAllCurrentSessionNames(UObject* Context);
	
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Extra", meta=( WorldContext = "Context" ))
	static FString GetProductUserID(UObject* Context);


	/** Lobby Voice Functions START */

	static IVoiceChatUser* GetLobbyVoiceChat(UObject* Context);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool MuteLobbyVoiceChat(UObject* Context, bool bMute);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool IsLobbyVoiceChatMuted(UObject* Context);
	
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool SetLobbyOutputMethod(UObject* Context, FString MethodID);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool SetLobbyInputMethod(UObject* Context, FString MethodID);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool BlockLobbyVoiceChatPlayers(UObject* Context, TArray<FString> BlockedPlayers);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool UnblockLobbyVoiceChatPlayers(UObject* Context, TArray<FString> UnblockedPlayers);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static float GetLobbyVoiceChatOutputVolume(UObject* Context);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool SetLobbyVoiceChatOutputVolume(UObject* Context, float Volume);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool SetLobbyVoiceChatInputVolume(UObject* Context, float Volume);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static float GetLobbyVoiceChatInputVolume(UObject* Context);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static bool SetLobbyPlayerVoiceChatVolume(UObject* Context, FString PlayerName, float Volume);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Lobby Voice", meta=( WorldContext = "Context" ))
	static float GetLobbyPlayerVoiceChatVolume(UObject* Context, FString PlayerName);

	/** Voice Chat Functions END */

	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 || Friends")
	static bool ShowFriendsList();

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || UserInfo")
	static FEOKUniqueNetId MakeEOKUniqueNetId(FString EpicAccountId, FString ProductUserId);

	UFUNCTION(BlueprintCallable, DisplayName="Accept EOK Session Invite", Category="Epic Online Services-Kit V2 || Sessions")
	static bool AcceptSessionInvite(FString InviteId, FString LocalUserId, FString InviterUserId);

	UFUNCTION(BlueprintCallable, DisplayName="Reject EOK Session Invite", Category="Epic Online Services-Kit V2 || Sessions")
	static bool RejectSessionInvite(FString InviteId, FString LocalUserId);
	
	// This is a C++ method definition for starting lobbies and sessions
	UFUNCTION(BlueprintCallable, DisplayName="Start EOK Session", Category="Epic Online Services-Kit V2 || Sessions")
	static bool StartSession(FName SessionName = "GameSession");

	// This is a C++ method definition for registering players in lobbies and sessions
	UFUNCTION(BlueprintCallable, DisplayName="Register EOK Player In Session",	Category="Epic Online Services-Kit V2 || Sessions")
	static bool RegisterPlayer(FName SessionName, FEOKUniqueNetId PlayerId, bool bWasInvited = false);

	// This is a C++ method definition for unregistering players from lobbies and sessions
	UFUNCTION(BlueprintCallable, DisplayName="Unregister EOK Player In Session",
		Category="Epic Online Services-Kit V2 || Sessions")
	static bool UnRegisterPlayer(FName SessionName, FEOKUniqueNetId PlayerId);

	// This is a C++ method definition for ending lobbies and sessions
	UFUNCTION(BlueprintCallable, DisplayName="End EOK Session", Category="Epic Online Services-Kit V2 || Sessions")
	static bool EndSession(FName SessionName = "GameSession");

	// This is a C++ method definition for checking if user is in a lobby or session
	UFUNCTION(BlueprintCallable, DisplayName="Is In EOK Session", Category="Epic Online Services-Kit V2 || Sessions")
	static bool IsInSession(FName SessionName, FEOKUniqueNetId PlayerId);

	// This is a C++ method definition for getting the nickname of a player from an online subsystem.
	// Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/extra-functions/getplayernickname
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 || Extra")
	static FString GetPlayerNickname(const int32 LocalUserNum);

	// This is a C++ method definition for getting the login status of a player from an online subsystem.
	// Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/extra-functions/getloginstatus
	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 || Extra")
	static EEOK_LoginStatus GetLoginStatus(const int32 LocalUserNum);

	// Generate a session code of the specified length.
	// This function returns a randomly generated alphanumeric session code.
	// The 'CodeLength' parameter determines the length of the generated code (default: 9).
	UFUNCTION(BlueprintPure, Category="Epic Online Services-Kit V2 || Extra")
	static FString GenerateSessionCode(int32 CodeLength = 9);

	// Check if EOS (Epic Online Services) is active or not.
	// This function returns true if EOS is active, false otherwise.
	UFUNCTION(BlueprintPure, Category="Epic Online Services-Kit V2 || Extra")
	static bool IsEOKActive();

	// Get the active EOS subsystem name.
	// This function returns the name of the active EOS subsystem as a FName.
	UFUNCTION(BlueprintPure, Category="Epic Online Services-Kit V2 || Extra")
	static FName GetActiveSubsystem();

	// Convert a TArray<uint8> to a FString using Base64 encoding.
	// This is useful for converting binary data to a human-readable string format.
	UFUNCTION(BlueprintPure, Category="Epic Online Services-Kit V2 || Extra || Conversions")
	static FString ByteArrayToString(const TArray<uint8>& DataToConvert);

	// Convert an FString to a TArray<uint8> using Base64 decoding.
	// This is useful for converting a string back to its original binary data representation.
	UFUNCTION(BlueprintPure, Category="Epic Online Services-Kit V2 || Extra || Conversions")
	static TArray<uint8> StringToByteArray(const FString& DataToConvert);

	// Convert a TArray<uint8> to a SaveGame object.
	// The TArray<uint8> is deserialized to construct the SaveGame object.
	UFUNCTION(BlueprintPure, Category="Epic Online Services-Kit V2 || Extra || Conversions")
	static USaveGame* ByteArrayToSaveGameObject(const TArray<uint8>& DataToConvert)
	{
		return UGameplayStatics::LoadGameFromMemory(DataToConvert);
	}

	// Convert a SaveGame object to a TArray<uint8>.
	// The SaveGame object is serialized into a binary data array (TArray<uint8>).
	UFUNCTION(BlueprintPure, Category="Epic Online Services-Kit V2 || Extra || Conversions")
	static TArray<uint8> SaveGameObjectToByteArray(USaveGame* DataToConvert)
	{
		TArray<uint8> Result;
		UGameplayStatics::SaveGameToMemory(DataToConvert, Result);
		return Result;
	}

	// This is a C++ method definition for getting the User Unique NetID
	// Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/extra-functions/get-useruniqueid
	UFUNCTION(BlueprintPure, DisplayName="Get User Unique NetID", Category="Epic Online Services-Kit V2 || Extra")
	static FEOKUniqueNetId GetUserUniqueID(const APlayerController* PlayerController, bool& bIsValid);

	UFUNCTION(BlueprintPure, DisplayName="Get User Unique NetID From PlayerState", Category="Epic Online Services-Kit V2 || Extra")
	static FEOKUniqueNetId GetUserUniqueIDFromPlayerState(const APlayerState* PlayerState, bool& bIsValid);

	UFUNCTION(BlueprintPure, DisplayName="EOS SDK Version", Category="Epic Online Services-Kit V2 || Extra")
	static FString GetEOSSDKVersion();

	//Get Plugin Version
	UFUNCTION(BlueprintPure, DisplayName="Get EOK Plugin Version", Category="Epic Online Services-Kit V2 || Extra")
	static FString GetEOKPluginVersion();

	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 || Extra")
	static bool IsValidSession(FSessionFindStruct Session);

	UFUNCTION(BlueprintPure, Category="Epic Online Services-Kit V2 || Extra")
	static FString GetCurrentPort(AGameModeBase* CurrentGameMode);

	UFUNCTION(BlueprintCallable, Category="Epic Online Services-Kit V2 || Extra")
	static void MakeSanctionAppeal(FString AccessToken, EEOSSanctionType Reason,
	                               const FOnResponseFromSanctions& OnResponseFromSanctions);

	UFUNCTION(BlueprintCallable, DisplayName="Request EOS Access Token", Category="Epic Online Services-Kit V2 || Extra")
	static void RequestEOSAccessToken(const FOnResponseFromEpicForAccessToken& Response);

	UFUNCTION(BlueprintCallable, DisplayName="Convert POSIX Time to DateTime", Category="Epic Online Services-Kit V2 || Extra")
	static FDateTime ConvertPosixTimeToDateTime(int64 PosixTime);

	UFUNCTION(BlueprintCallable, DisplayName="Get Resolved Connect String", Category="Epic Online Services-Kit V2 || Extra")
	static FString GetResolvedConnectString(FName SessionName);

	UFUNCTION(BlueprintCallable, DisplayName="Get AutoLogin Attempted Status", Category="Epic Online Services-Kit V2 || Extra")
	static bool GetAutoLoginAttemptedStatus();

	UFUNCTION(BlueprintCallable, DisplayName="Get AutoLogin In Progress Status", Category="Epic Online Services-Kit V2 || Extra")
	static bool GetAutoLoginInProgressStatus();

	UFUNCTION(BlueprintCallable, DisplayName="Get Environment Variable", Category="Epic Online Services-Kit V2 || Extra")
	static FString GetEnvironmentVariable(const FString& EnvVariableName);

	UFUNCTION(BlueprintCallable, DisplayName="Init Ping Beacon", Category="Epic Online Services-Kit V2 || Extra", meta=(WorldContext="Context"))
	static bool InitPingBeacon(UObject* Context, AGameModeBase* GameMode);

};
