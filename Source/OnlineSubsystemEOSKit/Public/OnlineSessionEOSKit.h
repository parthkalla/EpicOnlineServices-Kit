// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemEOSKit.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemEOSKitPrivate.h"

#if WITH_EOS_SDK
	#include "eos_common.h"
	#include "eos_sessions.h"
	#include "eos_sessions_types.h"
	#include "eos_lobby.h"
	#include "eos_lobby_types.h"
	
	// Forward declare FUniqueNetIdEOSLobby (defined in cpp like EIK)
	class FUniqueNetIdEOSLobby;
	// Forward declare ref type so it can be used in header signatures (use const to match TEMP_UNIQUENETIDSTRING_SUBCLASS)
	using FUniqueNetIdEOSLobbyRef = TSharedRef<const FUniqueNetIdEOSLobby>;
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK
// Forward declare structs (defined in cpp)
struct FSessionSearchHandleWrapper;

// Lobby details wrapper (similar to EIK)
struct FLobbyDetailsEOSKit : FNoncopyable
{
	EOS_HLobbyDetails LobbyDetailsHandle;

	FLobbyDetailsEOSKit(EOS_HLobbyDetails InLobbyDetailsHandle)
		: LobbyDetailsHandle(InLobbyDetailsHandle)
	{
	}

	virtual ~FLobbyDetailsEOSKit()
	{
		if (LobbyDetailsHandle)
		{
			EOS_LobbyDetails_Release(LobbyDetailsHandle);
		}
	}
};
#endif

class FOnlineSessionInfoEOSKit : public FOnlineSessionInfo
{
public:
	FOnlineSessionInfoEOSKit()
		: SessionId(TEXT(""))
		, HostAddr(TEXT("127.0.0.1"))
		, SessionHandle(nullptr)
	{
		SessionIdUnique = FUniqueNetIdString::Create(SessionId, FName(TEXT("EOS")));
	}

	explicit FOnlineSessionInfoEOSKit(const FString& InHostAddr)
		: SessionId(TEXT(""))
		, HostAddr(InHostAddr)
		, SessionHandle(nullptr)
	{
		SessionIdUnique = FUniqueNetIdString::Create(SessionId, FName(TEXT("EOS")));
	}

	// Copy constructor
	FOnlineSessionInfoEOSKit(const FOnlineSessionInfoEOSKit& Other)
		: SessionId(Other.SessionId)
		, HostAddr(Other.HostAddr)
		, EOSAddress(Other.EOSAddress) // CRITICAL: Copy EOSAddress for P2P connections
		, SessionHandle(Other.SessionHandle)
	{
		SessionIdUnique = FUniqueNetIdString::Create(SessionId, FName(TEXT("EOS")));
	}

	FString SessionId;
	FString HostAddr;
	FString EOSAddress; // EOS P2P connection string (format: EOS:ProductUserId:SocketName:Channel)
	FUniqueNetIdPtr SessionIdUnique;
	
#if WITH_EOS_SDK
	// EOS session details handle (required for joining sessions)
	EOS_HSessionDetails SessionHandle;
#else
	void* SessionHandle;
#endif

	void SetSessionId(const FString& InSessionId)
	{
		SessionId = InSessionId;
		SessionIdUnique = FUniqueNetIdString::Create(InSessionId, FName(TEXT("EOS")));
	}

	virtual const uint8* GetBytes() const override { return nullptr; }
	virtual int32 GetSize() const override { return 0; }
	virtual bool IsValid() const override { return !SessionId.IsEmpty(); }
	virtual FString ToString() const override { return SessionId; }
	virtual FString ToDebugString() const override { return FString::Printf(TEXT("%s (%s)"), *SessionId, *HostAddr); }
	virtual const FUniqueNetId& GetSessionId() const override { check(SessionIdUnique.IsValid()); return *SessionIdUnique; }
};

#if WITH_EOS_SDK

/**
 * Interface for interacting with EOS sessions
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineSessionEOSKit :
	public IOnlineSession
	, public TSharedFromThis<FOnlineSessionEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineSessionEOSKit() = delete;
	explicit FOnlineSessionEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineSessionEOSKit();

	// IOnlineSession interface
	virtual FUniqueNetIdPtr CreateSessionIdFromString(const FString& SessionIdStr) override;
	virtual FNamedOnlineSession* GetNamedSession(FName SessionName) override;
	const FNamedOnlineSession* GetNamedSession(FName SessionName) const;
	virtual void RemoveNamedSession(FName SessionName) override;
	virtual bool HasPresenceSession() override { return true; }
	virtual EOnlineSessionState::Type GetSessionState(FName SessionName) const override;
	virtual bool CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override;
	virtual bool CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override;
	virtual bool StartSession(FName SessionName) override;
	virtual bool UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData = false) override;
	virtual bool EndSession(FName SessionName) override;
	virtual bool DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& Delegate = FOnDestroySessionCompleteDelegate()) override;
	virtual bool IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId) override;
	virtual bool StartMatchmaking(const TArray<FUniqueNetIdRef>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	virtual bool CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName) override;
	virtual bool CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName) override;
	virtual bool FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	virtual bool FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	virtual bool FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& Delegate) override;
	virtual bool CancelFindSessions() override;
	virtual bool PingSearchResults(const FOnlineSessionSearchResult& SearchResult) override;
	virtual bool JoinSession(int32 PlayerNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override;
	virtual bool JoinSession(const FUniqueNetId& PlayerId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override;
	virtual bool FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend) override;
	virtual bool FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& FriendId) override;
	virtual bool FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<FUniqueNetIdRef>& Friends) override;
	virtual bool SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend) override;
	virtual bool SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& FriendId) override;
	virtual bool SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray<FUniqueNetIdRef>& Friends) override;
	virtual bool SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray<FUniqueNetIdRef>& FriendIds) override;
	virtual bool GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType = NAME_GamePort) override;
	virtual bool GetResolvedConnectString(const FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo) override;
	virtual FOnlineSessionSettings* GetSessionSettings(FName SessionName) override;
	virtual bool RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited) override;
	virtual bool RegisterPlayers(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasInvited) override;
	virtual bool UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId) override;
	virtual bool UnregisterPlayers(FName SessionName, const TArray<FUniqueNetIdRef>& Players) override;
	virtual int32 GetNumSessions() override;
	virtual void DumpSessionState() override;
	virtual FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSession& Session) override;
	virtual FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSessionSettings& SessionSettings) override;
	virtual void RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate) override;
	virtual void UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate) override;

	// Wrapper methods to delegate to existing EOSKit subsystems
	void DelegateToEOSKitSessionManager();
	
	// Initialize callbacks after construction (public so it can be called from factory)
	void RegisterSessionInviteCallbacks();

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HSessions SessionsHandle;
	EOS_HLobby LobbyHandle; // Lobby interface handle
	
	/** Lock for session access */
	mutable FCriticalSection SessionLock;
	
	/** Active sessions */
	TArray<FNamedOnlineSession> Sessions;
	
	/** Active session searches */
	TSharedPtr<FOnlineSessionSearch> CurrentSessionSearch;
	
	/** EOS handle wrapper to hold onto it for scope of the search (from EIK) */
	TSharedPtr<struct FSessionSearchHandleWrapper> CurrentSearchHandle;
	
	/** Last invite search (for session invites) */
	TSharedPtr<FOnlineSessionSearch> LastInviteSearch;
	
	/** Lobby search results cache (maps LobbyId to LobbyDetails handle) */
	TMap<FString, TSharedRef<FLobbyDetailsEOSKit>> LobbySearchResultsCache;
	TArray<TSharedRef<FLobbyDetailsEOSKit>> PendingLobbySearchResults;
	double SessionSearchStartInSeconds;
	
	// Session invite notification IDs
	EOS_NotificationId SessionInviteAcceptedId;
	
	// LAN session support (from EIK)
	class FLANSession;
	TSharedPtr<FLANSession> LANSession;
	
	// Dedicated server and P2P socket flags (from EIK)
	bool bIsDedicatedServer;
	bool bIsUsingP2PSockets;
	
	// Lobby callback pointers (from EIK) - using void* to avoid typedef forward declaration issues
	// These are cast to the proper types in the cpp file where the typedefs are defined
	void* LobbyCreatedCallback;
	void* LobbyUpdatedCallback;
	void* LobbyJoinedCallback;
	void* LobbyLeftCallback;
	void* LobbyDestroyedCallback;
	void* LobbySendInviteCallback;
	void* LobbySearchFindCallback;
	void* LobbyUpdateReceivedCallback;
	void* LobbyMemberUpdateReceivedCallback;
	void* LobbyMemberStatusReceivedCallback;
	void* LobbyInviteAcceptedCallback;
	void* JoinLobbyAcceptedCallback;
	void* LeaveLobbyRequestCallback;
	
	// Session invite callback pointers (from EIK)
	void* SessionInviteAcceptedCallback;

	// Delegate instances (kept as internal members; not exposed as IOnlineSession overrides)
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FOnUpdateSessionCompleteDelegate OnUpdateSessionCompleteDelegate;
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FOnCancelFindSessionsCompleteDelegate OnCancelFindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnRegisterLocalPlayerCompleteDelegate OnRegisterLocalPlayerCompleteDelegate;
	FOnUnregisterLocalPlayerCompleteDelegate OnUnregisterLocalPlayerCompleteDelegate;

	// EOS Session functions
	uint32 CreateEOSSessionInternal(int32 HostingPlayerNum, FNamedOnlineSession* Session);
	uint32 FindEOSSession(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings);
	void FindEOSSessionById(int32 LocalUserNum, const FUniqueNetId& SessionId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate);
	uint32 JoinEOSSession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession);
	uint32 UpdateEOSSession(FNamedOnlineSession* Session);
	uint32 EndEOSSession(FNamedOnlineSession* Session);
	uint32 DestroyEOSSession(FNamedOnlineSession* Session, const FOnDestroySessionCompleteDelegate& CompletionDelegate);
	
	// LAN Session functions (from EIK)
	uint32 CreateLANSession(int32 HostingPlayerNum, FNamedOnlineSession* Session);
	uint32 JoinLANSession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession);
	uint32 FindLANSession();
	void TickLanTasks(float DeltaTime);
	void AppendSessionToPacket(class FNboSerializeToBufferEOS& Packet, class FOnlineSession* Session);
	void AppendSessionSettingsToPacket(class FNboSerializeToBufferEOS& Packet, FOnlineSessionSettings* SessionSettings);
	void ReadSessionFromPacket(class FNboSerializeFromBufferEOS& Packet, class FOnlineSession* Session);
	void ReadSettingsFromPacket(class FNboSerializeFromBufferEOS& Packet, FOnlineSessionSettings& SessionSettings);
	void OnValidQueryPacketReceived(uint8* PacketData, int32 PacketLength, uint64 ClientNonce);
	void OnValidResponsePacketReceived(uint8* PacketData, int32 PacketLength);
	void OnLANSearchTimeout();
	
	// EOS Lobby functions
	uint32 CreateLobbySession(int32 HostingPlayerNum, FNamedOnlineSession* Session);
	uint32 FindLobbySession(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings);
	void StartLobbySearch(int32 SearchingPlayerNum, EOS_HLobbySearch LobbySearchHandle, const TSharedRef<FOnlineSessionSearch>& SearchSettings, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate);
	uint32 JoinLobbySession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession);
	uint32 UpdateLobbySession(FNamedOnlineSession* Session);
	uint32 StartLobbySession(FNamedOnlineSession* Session);
	uint32 EndLobbySession(FNamedOnlineSession* Session);
	uint32 DestroyLobbySession(FNamedOnlineSession* Session, const FOnDestroySessionCompleteDelegate& CompletionDelegate);
	
	// Lobby helper functions for session lookup
	FNamedOnlineSession* GetNamedSessionFromLobbyId(const FUniqueNetIdEOSLobby& LobbyId);
	FOnlineSessionSearchResult* GetSearchResultFromLobbyId(const FUniqueNetIdEOSLobby& LobbyId);
	FOnlineSession* GetOnlineSessionFromLobbyId(const FUniqueNetIdEOSLobby& LobbyId);
	
	// Lobby member management
	// Note: FUniqueNetIdEOSLobby is defined in cpp file; pass shared ref explicitly to avoid typedef conflicts
	void UpdateOrAddLobbyMember(const TSharedRef<const FUniqueNetIdEOSLobby>& LobbyNetId, const FUniqueNetIdRef& PlayerId);
	bool AddOnlineSessionMember(FName SessionName, const FUniqueNetIdRef& PlayerId);
	bool RemoveOnlineSessionMember(FName SessionName, const FUniqueNetIdRef& PlayerId);
	
	// ProductUserId resolution helpers
	FUniqueNetIdPtr ResolveProductUserIdToUniqueNetId(EOS_ProductUserId ProductUserId) const;
	void ResolveProductUserIdsToUniqueNetIds(const TArray<EOS_ProductUserId>& ProductUserIds, TFunction<void(TMap<EOS_ProductUserId, FUniqueNetIdRef>)> Callback) const;
	FString ProductUserIdToString(EOS_ProductUserId ProductUserId) const;
	
	// Lobby helper functions
	void SetLobbyPermissionLevel(EOS_HLobbyModification LobbyModificationHandle, FNamedOnlineSession* Session);
	void SetLobbyMaxMembers(EOS_HLobbyModification LobbyModificationHandle, FNamedOnlineSession* Session);
	void SetLobbyAttributes(EOS_HLobbyModification LobbyModificationHandle, FNamedOnlineSession* Session);
	void AddLobbyAttribute(EOS_HLobbyModification LobbyModificationHandle, const EOS_Lobby_AttributeData* Attribute);
	void AddLobbyMemberAttribute(EOS_HLobbyModification LobbyModificationHandle, const EOS_Lobby_AttributeData* Attribute);
	
	// Lobby data copying functions
	typedef TFunction<void(bool bWasSuccessful)> FOnCopyLobbyDataCompleteCallback;
	void CopyLobbyData(const TSharedRef<FLobbyDetailsEOSKit>& LobbyDetails, EOS_LobbyDetails_Info* LobbyDetailsInfo, FOnlineSession& OutSession, const FOnCopyLobbyDataCompleteCallback& Callback);
	void CopyLobbyAttributes(const TSharedRef<FLobbyDetailsEOSKit>& LobbyDetails, FOnlineSession& OutSession);
	void CopyLobbyMemberAttributes(const FLobbyDetailsEOSKit& LobbyDetails, const EOS_ProductUserId& TargetUserId, FSessionSettings& OutSessionSettings);
	void AddLobbySearchResult(const TSharedRef<FLobbyDetailsEOSKit>& LobbyDetails, const TSharedRef<FOnlineSessionSearch>& SearchSettings, const FOnCopyLobbyDataCompleteCallback& Callback);
	void AddLobbySearchAttribute(EOS_HLobbySearch LobbySearchHandle, const EOS_Lobby_AttributeData* Attribute, EOS_EOnlineComparisonOp ComparisonOp);
	
	// Lobby notification callbacks
	EOS_NotificationId LobbyUpdateReceivedId;
	EOS_NotificationId LobbyMemberUpdateReceivedId;
	EOS_NotificationId LobbyMemberStatusReceivedId;
	EOS_NotificationId LobbyInviteAcceptedId;
	EOS_NotificationId JoinLobbyAcceptedId;
#if PLATFORM_WINDOWS
	EOS_NotificationId LeaveLobbyRequestId;
#endif
	
	void RegisterLobbyNotifications();
	void OnLobbyUpdateReceived(const EOS_LobbyId& LobbyId);
	void OnLobbyMemberUpdateReceived(const EOS_LobbyId& LobbyId, const EOS_ProductUserId& TargetUserId);
	void OnMemberStatusReceived(const EOS_LobbyId& LobbyId, const EOS_ProductUserId& TargetUserId, EOS_ELobbyMemberStatus CurrentStatus);
	void OnLobbyInviteAccepted(const char* InviteId, const EOS_ProductUserId& LocalUserId, const EOS_ProductUserId& TargetUserId);
	void OnJoinLobbyAccepted(const EOS_ProductUserId& LocalUserId, const EOS_UI_EventId& UiEventId);
#if PLATFORM_WINDOWS
	void OnLeaveLobbyRequested(const EOS_ProductUserId& LocalUserId, const EOS_Lobby_LeaveLobbyRequestedCallbackInfo* Data);
#endif
	
	// Helper functions
	void Init(const FString& InBucketId);
	void RegisterLocalPlayers(FNamedOnlineSession* Session);
	void HandleCreateSessionCallback(FName SessionName, const EOS_Sessions_UpdateSessionCallbackInfo* Data);
	EOS_ProductUserId GetProductUserIdForSession(FNamedOnlineSession* Session) const;
	EOS_ELobbyPermissionLevel GetLobbyPermissionLevelFromSessionSettings(const FOnlineSessionSettings& SessionSettings);
	uint32_t GetLobbyMaxMembersFromSessionSettings(const FOnlineSessionSettings& SessionSettings);
	
	// Session search helpers
	void AddSearchAttribute(EOS_HSessionSearch SearchHandle, const EOS_Sessions_AttributeData* Attribute, EOS_EOnlineComparisonOp ComparisonOp);
	void AddSearchResult(EOS_HSessionDetails SessionHandle, const TSharedRef<FOnlineSessionSearch>& SearchSettings);
	void CopySearchResult(EOS_HSessionDetails SessionHandle, EOS_SessionDetails_Info* SessionInfo, FOnlineSession& OutSession);
	void CopyAttributes(EOS_HSessionDetails SessionHandle, FOnlineSession& OutSession);
	EOS_EOnlineComparisonOp ToEOSSearchOp(EOnlineComparisonOp::Type Op);
	bool IsSessionSettingTypeSupported(EOnlineKeyValuePairDataType::Type InType);

	// Session modification helpers (from EIK)
	void SetPermissionLevel(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session);
	void SetMaxPlayers(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session);
	void SetInvitesAllowed(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session);
	void SetJoinInProgress(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session);
	void AddAttribute(EOS_HSessionModification SessionModHandle, const EOS_Sessions_AttributeData* Attribute);
	void SetAttributes(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session);
	uint32 SharedSessionUpdate(EOS_HSessionModification SessionModHandle, FNamedOnlineSession* Session, void* InCallback);

	// Analytics functions (from EIK)
	void BeginSessionAnalytics(FNamedOnlineSession* Session);
	void EndSessionAnalytics();

	// Send invite functions (from EIK)
	bool SendSessionInvite(FName SessionName, EOS_ProductUserId SenderId, EOS_ProductUserId ReceiverId);
	bool SendLobbyInvite(FName SessionName, EOS_ProductUserId SenderId, EOS_ProductUserId ReceiverId);
	bool SendEOSSessionInvite(FName SessionName, EOS_ProductUserId SenderId, EOS_ProductUserId ReceiverId);

	// Connect string helpers (from EIK)
	bool GetConnectStringFromSessionInfoForBeacon(TSharedPtr<FOnlineSessionInfoEOSKit>& SessionInfo, FString& ConnectInfo, int32 PortOverride = 0);

	// Helper functions (from EIK)
	bool IsHost(const FNamedOnlineSession& Session) const;

	/** Auto travel helper (mirrors EIK behavior) */
	void TravelToSession(FName SessionName);
};

#endif // WITH_EOS_SDK

