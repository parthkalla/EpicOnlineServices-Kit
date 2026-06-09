// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_FindSessions_AsyncFunction.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/OnlineSessionNames.h"
#endif

UEOK_FindSessions_AsyncFunction* UEOK_FindSessions_AsyncFunction::FindEOKSessions(
	TMap<FString, FEOKAttribute> SessionSettings, EMatchType MatchType, int32 MaxResults, ERegionInfo RegionToSearch, bool bLanSearch, bool bIncludePartySessions)

{
	UEOK_FindSessions_AsyncFunction* UEOK_FindSessionObject= NewObject<UEOK_FindSessions_AsyncFunction>();
	UEOK_FindSessionObject->E_MatchType = MatchType;
	UEOK_FindSessionObject->E_RegionToSearch = RegionToSearch;
	UEOK_FindSessionObject->SessionSettings = SessionSettings;
	UEOK_FindSessionObject->B_bLanSearch = bLanSearch;
	UEOK_FindSessionObject->I_MaxResults = MaxResults;
	UEOK_FindSessionObject->bIncludePartySessions = bIncludePartySessions;
	return UEOK_FindSessionObject;
}

void UEOK_FindSessions_AsyncFunction::Activate()
{
	FindSession();
	Super::Activate();
}

void UEOK_FindSessions_AsyncFunction::FindSession()
{
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get("EOK"))
	{
		if(const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->QuerySettings.SearchParams.Empty();
			SessionSearch->bIsLanQuery = B_bLanSearch;
			if(E_MatchType == EMatchType::MT_MatchMakingSession)
			{
				if(E_RegionToSearch!=ERegionInfo::RE_NoSelection)
				{
					SessionSearch->QuerySettings.Set(FName(TEXT("RegionInfo")),UEnum::GetValueAsString(E_RegionToSearch), EOnlineComparisonOp::Equals);
				}
			}
			else
			{
				SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
				if(!bIncludePartySessions)
				{
					SessionSearch->QuerySettings.Set("IsPartySession", false, EOnlineComparisonOp::Equals);
				}
			}
			if (SessionSettings.Num() > 0) {
				for (auto& Settings_SingleValue : SessionSettings) {
					if (Settings_SingleValue.Key.IsEmpty()) {
						continue;
					}
					FOnlineSessionSetting Setting;
					Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
					Setting.Data = Settings_SingleValue.Value.GetVariantData();
					FName SettingName = FName(*Settings_SingleValue.Key);
					if(Settings_SingleValue.Value.AttributeType == EEOKAttributeType::String)
					{
						SessionSearch->QuerySettings.Set(SettingName, Settings_SingleValue.Value.StringValue, EOnlineComparisonOp::Equals);
					}
					else if(Settings_SingleValue.Value.AttributeType == EEOKAttributeType::Integer)
					{
						SessionSearch->QuerySettings.Set(SettingName, Settings_SingleValue.Value.IntValue, EOnlineComparisonOp::Equals);
					}
					else if(Settings_SingleValue.Value.AttributeType == EEOKAttributeType::Bool)
					{
						SessionSearch->QuerySettings.Set(SettingName, Settings_SingleValue.Value.BoolValue, EOnlineComparisonOp::Equals);
					}
				}
			}
			SessionSearch->MaxSearchResults = I_MaxResults;
			SessionPtrRef->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOK_FindSessions_AsyncFunction::OnFindSessionCompleted);
			SessionPtrRef->FindSessions(0,SessionSearch.ToSharedRef());
		}
		else
		{
			if(bDelegateCalled)
			{
				return;
			}
			OnFail.Broadcast(TArray<FSessionFindStruct>());
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	else
	{
		if(bDelegateCalled)
		{
			return;
		}
		OnFail.Broadcast(TArray<FSessionFindStruct>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}

void UEOK_FindSessions_AsyncFunction::OnFindSessionCompleted(bool bWasSuccess)
{
	if(!bWasSuccess)
	{
		if(bDelegateCalled)
		{
			return;
		}
		OnFail.Broadcast(TArray<FSessionFindStruct>());
		bDelegateCalled = true;
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
	else
	{
		if (const IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
		{
			TArray<FSessionFindStruct> SessionResult_Array;
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				if (SessionSearch->SearchResults.Num() > 0)
				{
					for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
					{
						FBlueprintSessionResult SessionResult;
						SessionResult.OnlineResult = SessionSearch->SearchResults[SearchIdx];
						FOnlineSessionSettings LocalSessionSettings = SessionResult.OnlineResult.Session.SessionSettings;
						TMap<FName, FString> AllSettingsWithData;
						TMap<FName, FOnlineSessionSetting>::TIterator It(LocalSessionSettings.Settings);

						TMap<FString, FEOKAttribute> LocalArraySettings;
						FSessionFindStruct LocalStruct;
						while (It)
						{
							if(It.Key().ToString() == "SessionName")
							{
								LocalStruct.SessionName = It.Value().Data.ToString();
							}
							const FName& SettingName = It.Key();
							const FOnlineSessionSetting& Setting = It.Value();
							LocalArraySettings.Add(*SettingName.ToString(), Setting.Data);
							++It;
						}
						if(LocalStruct.SessionName.IsEmpty())
						{
							LocalStruct.SessionName = "GameSession";
						}
						bool IsServer = LocalArraySettings.Contains("IsDedicatedServer") ? true : false;
						LocalStruct.CurrentNumberOfPlayers = (SessionResult.OnlineResult.Session.SessionSettings.NumPublicConnections + SessionResult.OnlineResult.Session.SessionSettings.NumPrivateConnections) - (SessionResult.OnlineResult.Session.NumOpenPublicConnections + SessionResult.OnlineResult.Session.NumOpenPrivateConnections);
						LocalStruct.MaxNumberOfPlayers = SessionResult.OnlineResult.Session.SessionSettings.NumPublicConnections + SessionResult.OnlineResult.Session.SessionSettings.NumPrivateConnections;
						LocalStruct.SessionResult = SessionResult;
						LocalStruct.SessionSettings = LocalArraySettings;
						LocalStruct.bIsDedicatedServer = IsServer;
						SessionResult_Array.Add(LocalStruct);
					}
				}
			}
			if (bDelegateCalled)
			{
				return;
			}
			bDelegateCalled = true;
			OnSuccess.Broadcast(SessionResult_Array);
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}
