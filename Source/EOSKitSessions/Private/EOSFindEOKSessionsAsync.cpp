// Copyright (C) 2024, All Rights Reserved.

#include "EOSFindEOKSessionsAsync.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

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
	
	// Use OnlineSubsystem interface (like EIK)
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			// Create search object
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = B_bLanSearch;
			SessionSearch->MaxSearchResults = I_MaxResults;
			
			// Add search filters
			for (const auto& Setting : SessionSettings)
			{
				if (Setting.Key.IsEmpty()) continue;
				
				FName SettingName = FName(*Setting.Key);
				if (!Setting.Value.StringValue.IsEmpty())
				{
					SessionSearch->QuerySettings.Set(SettingName, Setting.Value.StringValue, EOnlineComparisonOp::Equals);
				}
				else if (Setting.Value.IntValue != 0)
				{
					SessionSearch->QuerySettings.Set(SettingName, Setting.Value.IntValue, EOnlineComparisonOp::Equals);
				}
				else
				{
					SessionSearch->QuerySettings.Set(SettingName, Setting.Value.BoolValue, EOnlineComparisonOp::Equals);
				}
			}
			
			// Register callback
			SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSFindEOKSessionsAsync::OnFindSessionCompleted);
			
			// Start search
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Calling OnlineSubsystem->FindSessions()..."));
			SessionPtr->FindSessions(0, SessionSearch.ToSharedRef());
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
