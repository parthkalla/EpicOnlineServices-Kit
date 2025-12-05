// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemEOSKit.h"
#include "OnlineSubsystemTypes.h"

#if WITH_EOS_SDK
	#include "eos_sessions.h"
	#include "eos_sessions_types.h"
#endif

class FOnlineSubsystemEOSKit;

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

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HSessions SessionsHandle;
	
	/** Lock for session access */
	mutable FCriticalSection SessionLock;
	
	/** Active sessions */
	TArray<FNamedOnlineSession> Sessions;
	
	/** Active session searches */
	TSharedPtr<FOnlineSessionSearch> CurrentSessionSearch;

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

	uint32 CreateEOSSessionInternal(int32 HostingPlayerNum, FNamedOnlineSession* Session);
	void RegisterLocalPlayers(FNamedOnlineSession* Session);
	void HandleCreateSessionCallback(FName SessionName, const EOS_Sessions_UpdateSessionCallbackInfo* Data);
	EOS_ProductUserId GetProductUserIdForSession(FNamedOnlineSession* Session) const;
};

#endif // WITH_EOS_SDK

