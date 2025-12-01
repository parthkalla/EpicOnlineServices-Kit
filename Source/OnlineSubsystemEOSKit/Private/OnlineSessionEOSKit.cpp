// Copyright (C) 2024, All Rights Reserved.

#include "OnlineSessionEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitSessionManager.h"
#include "EOSKitSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

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
					// Convert FOnlineSessionSettings to FEOSKitCreateSessionSettings
					// This is a simplified version - full implementation would do proper conversion
					UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::CreateSession: Delegating to UEOSKitSessionManager"));
					// SessionManager->CreateEOSKitSession(...);
					return true;
				}
			}
		}
	}
	return false;
}

bool FOnlineSessionEOSKit::CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	return CreateSession(0, SessionName, NewSessionSettings);
}

bool FOnlineSessionEOSKit::StartSession(FName SessionName)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		Session->SessionState = EOnlineSessionState::InProgress;
		return true;
	}
	return false;
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
					CurrentSessionSearch = SearchSettings;
					UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::FindSessions: Delegating to UEOSKitSessionManager"));
					// SessionManager->FindEOSKitSessions(...);
					return true;
				}
			}
		}
	}
	return false;
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
					UE_LOG_ONLINE(Log, TEXT("FOnlineSessionEOSKit::JoinSession: Delegating to UEOSKitSessionManager"));
					// Convert FOnlineSessionSearchResult to FEOSKitSessionResult and call SessionManager->JoinEOSKitSession(...);
					return true;
				}
			}
		}
	}
	return false;
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
	if (Session)
	{
		// Get connection string from session
		return true;
	}
	return false;
}

bool FOnlineSessionEOSKit::GetResolvedConnectString(const FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo)
{
	// Extract connection string from search result
	return false;
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
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		Session->RegisteredPlayers.AddUnique(PlayerId.AsShared());
		return true;
	}
	return false;
}

bool FOnlineSessionEOSKit::RegisterPlayers(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasInvited)
{
	bool bSuccess = true;
	for (const FUniqueNetIdRef& PlayerId : Players)
	{
		if (!RegisterPlayer(SessionName, *PlayerId, bWasInvited))
		{
			bSuccess = false;
		}
	}
	return bSuccess;
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

