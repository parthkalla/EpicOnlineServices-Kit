// Copyright (C) 2024, All Rights Reserved.

#include "EOSFindEOKSessionsAsync.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

// SEARCH_LOBBIES constant (defined in OnlineSessionSettings.h in Unreal Engine)
#ifndef SEARCH_LOBBIES
static const FName SEARCH_LOBBIES(TEXT("SEARCH_LOBBIES"));
#endif

UEOSFindEOKSessionsAsync* UEOSFindEOKSessionsAsync::FindEOKSessions(
	UObject* WorldContextObject,
	TMap<FString, FEOSKitAttribute> SessionSettings,
	EEOSKitMatchType MatchType,
	int32 MaxResults,
	EEOSKitRegion RegionToSearch,
	bool bLanSearch,
	bool bIncludePartySessions)
{
	UEOSFindEOKSessionsAsync* Node = NewObject<UEOSFindEOKSessionsAsync>();
	Node->CachedWorldContextObject = WorldContextObject;
	Node->SessionSettings = SessionSettings;
	Node->E_MatchType = MatchType;
	Node->I_MaxResults = MaxResults;
	Node->E_RegionToSearch = RegionToSearch;
	Node->B_bLanSearch = bLanSearch;
	Node->bIncludePartySessions = bIncludePartySessions;
	return Node;
}

void UEOSFindEOKSessionsAsync::Activate()
{
	FindSession();
	Super::Activate();
}

void UEOSFindEOKSessionsAsync::FindSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Finding EOS Sessions via OnlineSubsystem (like EIK)"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Max Results: %d"), I_MaxResults);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Use OnlineSubsystem interface (exactly like EIK)
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->QuerySettings.SearchParams.Empty(); // Clear params like EIK does
			SessionSearch->bIsLanQuery = B_bLanSearch;
			
			// Match EIK's exact logic for MatchType
			if (E_MatchType == EEOSKitMatchType::MatchmakingSession)
			{
				// Matchmaking sessions - set RegionInfo if provided (like EIK)
				if (E_RegionToSearch != EEOSKitRegion::NoSelection)
				{
					FString RegionString = UEnum::GetValueAsString(E_RegionToSearch);
					SessionSearch->QuerySettings.Set(FName(TEXT("RegionInfo")), RegionString, EOnlineComparisonOp::Equals);
				}
				// Don't set SEARCH_LOBBIES for Matchmaking (searches regular sessions)
			}
			else
			{
				// Custom or Lobby sessions - search for Lobbies (like EIK)
				SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: MatchType is %d - searching for LOBBIES"), static_cast<int32>(E_MatchType));
				
				// Handle bIncludePartySessions like EIK
				if (!bIncludePartySessions)
				{
					SessionSearch->QuerySettings.Set(FName(TEXT("IsPartySession")), false, EOnlineComparisonOp::Equals);
				}
			}
			
			// Add search filters (exactly like EIK)
			if (SessionSettings.Num() > 0)
			{
				for (auto& Settings_SingleValue : SessionSettings)
				{
					if (Settings_SingleValue.Key.IsEmpty())
					{
						continue;
					}
					
					FName SettingName = FName(*Settings_SingleValue.Key);
					
					// Set value based on type (exactly like EIK)
					if (!Settings_SingleValue.Value.StringValue.IsEmpty())
					{
						SessionSearch->QuerySettings.Set(SettingName, Settings_SingleValue.Value.StringValue, EOnlineComparisonOp::Equals);
					}
					else if (Settings_SingleValue.Value.IntValue != 0)
					{
						SessionSearch->QuerySettings.Set(SettingName, Settings_SingleValue.Value.IntValue, EOnlineComparisonOp::Equals);
					}
					else
					{
						SessionSearch->QuerySettings.Set(SettingName, Settings_SingleValue.Value.BoolValue, EOnlineComparisonOp::Equals);
					}
				}
			}
			
			SessionSearch->MaxSearchResults = I_MaxResults;
			SessionPtrRef->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSFindEOKSessionsAsync::OnFindSessionCompleted);
			SessionPtrRef->FindSessions(0, SessionSearch.ToSharedRef());
			return;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: SessionInterface is null"));
			OnFail.Broadcast(TArray<FEOSKitSessionFindResult>());
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: OnlineSubsystem is null"));
		OnFail.Broadcast(TArray<FEOSKitSessionFindResult>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
}

void UEOSFindEOKSessionsAsync::OnFindSessionCompleted(bool bWasSuccess)
{
	TArray<FEOSKitSessionFindResult> Results;
	
	if (bWasSuccess && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Found %d sessions"), SessionSearch->SearchResults.Num());
		
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			FEOSKitSessionFindResult Result;
			
			// CRITICAL: Use a UNIQUE client session name to avoid "SessionAlreadyExists" error
			// Host uses "GameSession", clients use "ClientSession_<LocalPlayerNum>"
			// This way each client has a unique local session name but they all connect to the same EOS session
			Result.SessionName = TEXT("ClientSession");
			Result.SessionId = SearchResult.Session.SessionInfo.IsValid() ? SearchResult.Session.SessionInfo->GetSessionId().ToString() : TEXT("");
			
			// Calculate current players correctly
			// NumPublicConnections = max slots, NumOpenPublicConnections = available slots
			// CurrentPlayers = max - available
			Result.CurrentNumberOfPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
			Result.MaxNumberOfPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			Result.Ping = SearchResult.PingInMs;
			
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: FindSession Result - SessionID: %s, Players: %d/%d"), 
				*Result.SessionId, Result.CurrentNumberOfPlayers, Result.MaxNumberOfPlayers);
			
			// Store the search result for joining
			Result.OnlineResult = SearchResult;
			Result.SessionResult.OnlineResult = SearchResult;
			
			// Copy session settings
			for (const TPair<FName, FOnlineSessionSetting>& SettingPair : SearchResult.Session.SessionSettings.Settings)
			{
				FEOSKitAttribute Attr;
				const FOnlineSessionSetting& Setting = SettingPair.Value;
				
				switch (Setting.Data.GetType())
				{
				case EOnlineKeyValuePairDataType::String:
					{
						FString Value;
						Setting.Data.GetValue(Value);
						Attr.StringValue = Value;
						break;
					}
				case EOnlineKeyValuePairDataType::Bool:
					{
						bool bValue;
						Setting.Data.GetValue(bValue);
						Attr.BoolValue = bValue;
						break;
					}
				case EOnlineKeyValuePairDataType::Int32:
				case EOnlineKeyValuePairDataType::Int64:
					{
						int32 Value;
						Setting.Data.GetValue(Value);
						Attr.IntValue = Value;
						break;
					}
				default:
					break;
				}
				
				Result.SessionSettings.Add(SettingPair.Key.ToString(), Attr);
			}
			
			Results.Add(Result);
		}
		
		OnSuccess.Broadcast(Results);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: ❌ Failed to find sessions"));
		OnFail.Broadcast(Results);
	}
	
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
