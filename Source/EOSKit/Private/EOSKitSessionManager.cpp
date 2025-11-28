#include "EOSKitSessionManager.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

// Define missing settings constants if not already defined
#ifndef SETTING_MAPNAME
	#define SETTING_MAPNAME FName(TEXT("MAPNAME"))
#endif

#ifndef SETTING_GAMEMODE
	#define SETTING_GAMEMODE FName(TEXT("GAMEMODE"))
#endif

UEOSKitSessionManager::UEOSKitSessionManager()
{
	CurrentSessionName = NAME_GameSession;
}

void UEOSKitSessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get the online subsystem
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		
		if (SessionInterface.IsValid())
		{
			// Bind delegates
			OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
				FOnCreateSessionCompleteDelegate::CreateUObject(this, &UEOSKitSessionManager::OnCreateSessionCompleteInternal)
			);
			
			OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
				FOnFindSessionsCompleteDelegate::CreateUObject(this, &UEOSKitSessionManager::OnFindSessionsCompleteInternal)
			);
			
			OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
				FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSKitSessionManager::OnJoinSessionCompleteInternal)
			);
			
			OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
				FOnDestroySessionCompleteDelegate::CreateUObject(this, &UEOSKitSessionManager::OnDestroySessionCompleteInternal)
			);
			
			OnStartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(
				FOnStartSessionCompleteDelegate::CreateUObject(this, &UEOSKitSessionManager::OnStartSessionCompleteInternal)
			);
			
			OnEndSessionCompleteDelegateHandle = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(
				FOnEndSessionCompleteDelegate::CreateUObject(this, &UEOSKitSessionManager::OnEndSessionCompleteInternal)
			);

			UE_LOG(LogTemp, Log, TEXT("EOSKit Session Manager initialized successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Session Interface"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Online Subsystem not found"));
	}
}

void UEOSKitSessionManager::Deinitialize()
{
	// Clear delegates
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegateHandle);
	}

	Super::Deinitialize();
}

// ========== SESSION CREATION ==========

void UEOSKitSessionManager::CreateEOSKitSession(
	const FEOSKitCreateSessionSettings& SessionSettings,
	const FEOSKitDedicatedServerSettings& DedicatedServerSettings,
	const TMap<FString, FEOSKitAttribute>& ExtraSettings)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Interface not valid"));
		OnCreateSessionComplete.Broadcast(false);
		return;
	}

	// Destroy existing session if any
	auto ExistingSession = SessionInterface->GetNamedSession(CurrentSessionName);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(CurrentSessionName);
	}

	// Create session settings
	TSharedPtr<FOnlineSessionSettings> OnlineSessionSettings = MakeShareable(new FOnlineSessionSettings());
	SetupSessionSettings(*OnlineSessionSettings, SessionSettings);

	// Add extra settings
	for (const auto& Setting : ExtraSettings)
	{
		const FEOSKitAttribute& Attribute = Setting.Value;
		if (!Attribute.StringValue.IsEmpty())
		{
			OnlineSessionSettings->Set(FName(*Attribute.Key), Attribute.StringValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
		else if (Attribute.IntValue != 0)
		{
			OnlineSessionSettings->Set(FName(*Attribute.Key), Attribute.IntValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
		else if (Attribute.BoolValue)
		{
			OnlineSessionSettings->Set(FName(*Attribute.Key), Attribute.BoolValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
	}

	// Add dedicated server settings
	if (DedicatedServerSettings.bUseDedicatedServer)
	{
		OnlineSessionSettings->bIsDedicated = true;
		OnlineSessionSettings->Set(SETTING_MAPNAME, DedicatedServerSettings.DedicatedServerName, EOnlineDataAdvertisementType::ViaOnlineService);
	}

	// Create the session
	ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (LocalPlayer != nullptr)
	{
		bool bSuccess = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), CurrentSessionName, *OnlineSessionSettings);
		
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create session"));
			OnCreateSessionComplete.Broadcast(false);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Creating session '%s'..."), *SessionSettings.SessionName);
		}
	}
}

void UEOSKitSessionManager::CreateEOSKitLobby(
	const FEOSKitCreateLobbySettings& SessionSettings,
	const FEOSKitLobbyMemberSettings& MemberSettings,
	const TMap<FString, FEOSKitAttribute>& ExtraSettings)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Interface not valid"));
		OnCreateLobbyFailure.Broadcast();
		return;
	}

	// Destroy existing session if any
	auto ExistingSession = SessionInterface->GetNamedSession(CurrentSessionName);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(CurrentSessionName);
	}

	// Create lobby settings
	TSharedPtr<FOnlineSessionSettings> OnlineSessionSettings = MakeShareable(new FOnlineSessionSettings());
	SetupLobbySettings(*OnlineSessionSettings, SessionSettings);

	// Mark as lobby
	OnlineSessionSettings->Set(SETTING_GAMEMODE, FString("Lobby"), EOnlineDataAdvertisementType::ViaOnlineService);
	OnlineSessionSettings->bIsLANMatch = SessionSettings.bIsLanMatch;

	// Add extra settings
	for (const auto& Setting : ExtraSettings)
	{
		const FEOSKitAttribute& Attribute = Setting.Value;
		if (!Attribute.StringValue.IsEmpty())
		{
			OnlineSessionSettings->Set(FName(*Attribute.Key), Attribute.StringValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
		else if (Attribute.IntValue != 0)
		{
			OnlineSessionSettings->Set(FName(*Attribute.Key), Attribute.IntValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
		else if (Attribute.BoolValue)
		{
			OnlineSessionSettings->Set(FName(*Attribute.Key), Attribute.BoolValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
	}

	// Store lobby ID if provided
	if (!SessionSettings.LobbyIdOverride.IsEmpty())
	{
		CurrentLobbyId = SessionSettings.LobbyIdOverride;
	}

	// Create the lobby
	ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (LocalPlayer != nullptr)
	{
		bool bSuccess = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), CurrentSessionName, *OnlineSessionSettings);
		
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create lobby"));
			OnCreateLobbyFailure.Broadcast();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Creating lobby '%s'..."), *SessionSettings.SessionName);
		}
	}
}

// ========== SESSION FINDING ==========

void UEOSKitSessionManager::FindEOSKitSessions(const FEOSKitSessionSearchSettings& SessionSettings)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Interface not valid"));
		OnFindSessionsComplete.Broadcast(false, TArray<FEOSKitSessionResult>());
		return;
	}

	// Create search settings
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = SessionSettings.MaxResults;
	SessionSearch->bIsLanQuery = SessionSettings.bLanSearch;

	// Add search filters
	for (const auto& Filter : SessionSettings.SearchFilters)
	{
		const FEOSKitAttribute& Attribute = Filter.Value;
		if (!Attribute.StringValue.IsEmpty())
		{
			SessionSearch->QuerySettings.Set(FName(*Attribute.Key), Attribute.StringValue, EOnlineComparisonOp::Equals);
		}
		else if (Attribute.IntValue != 0)
		{
			SessionSearch->QuerySettings.Set(FName(*Attribute.Key), Attribute.IntValue, EOnlineComparisonOp::Equals);
		}
		else if (Attribute.BoolValue)
		{
			SessionSearch->QuerySettings.Set(FName(*Attribute.Key), Attribute.BoolValue, EOnlineComparisonOp::Equals);
		}
	}

	// Add match type filter
	FString MatchTypeString;
	switch (SessionSettings.MatchType)
	{
	case EEOSKitMatchType::MatchmakingSession:
		MatchTypeString = TEXT("MatchmakingSession");
		break;
	case EEOSKitMatchType::CustomSession:
		MatchTypeString = TEXT("CustomSession");
		break;
	case EEOSKitMatchType::LobbySession:
		MatchTypeString = TEXT("Lobby");
		break;
	}
	SessionSearch->QuerySettings.Set(SETTING_GAMEMODE, MatchTypeString, EOnlineComparisonOp::Equals);

	// Find sessions
	ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (LocalPlayer != nullptr)
	{
		bool bSuccess = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
		
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find sessions"));
			OnFindSessionsComplete.Broadcast(false, TArray<FEOSKitSessionResult>());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Searching for sessions..."));
		}
	}
}

void UEOSKitSessionManager::FindEOSKitLobbies(const FEOSKitSessionSearchSettings& SessionSettings)
{
	// Use the same function but with lobby match type
	FEOSKitSessionSearchSettings LobbySettings = SessionSettings;
	LobbySettings.MatchType = EEOSKitMatchType::LobbySession;
	FindEOSKitSessions(LobbySettings);
}

// ========== SESSION JOINING ==========

void UEOSKitSessionManager::JoinEOSKitSession(const FEOSKitSessionResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Interface not valid"));
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	// Find the search result
	if (!SessionSearch.IsValid() || SessionSearch->SearchResults.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: No search results available"));
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	// Find matching result
	FOnlineSessionSearchResult* FoundResult = nullptr;
	for (auto& Result : SessionSearch->SearchResults)
	{
		if (Result.GetSessionIdStr() == SessionResult.SessionId)
		{
			FoundResult = &Result;
			break;
		}
	}

	if (FoundResult == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session not found in search results"));
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	// Join the session
	ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (LocalPlayer != nullptr)
	{
		bool bSuccess = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), CurrentSessionName, *FoundResult);
		
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to join session"));
			OnJoinSessionComplete.Broadcast(false);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Joining session '%s'..."), *SessionResult.SessionName);
		}
	}
}

void UEOSKitSessionManager::JoinEOSKitLobby(const FEOSKitSessionResult& SessionResult)
{
	JoinEOSKitSession(SessionResult);
}

void UEOSKitSessionManager::JoinEOSKitLobbyById(const FString& LobbyId)
{
	// Store lobby ID and attempt to join
	CurrentLobbyId = LobbyId;
	
	// Create a temporary session result
	FEOSKitSessionResult TempResult;
	TempResult.SessionId = LobbyId;
	
	JoinEOSKitLobby(TempResult);
}

// ========== SESSION MANAGEMENT ==========

void UEOSKitSessionManager::DestroyEOSKitSession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Interface not valid"));
		OnDestroySessionComplete.Broadcast(false);
		return;
	}

	SessionInterface->DestroySession(CurrentSessionName);
}

void UEOSKitSessionManager::LeaveEOSKitLobby()
{
	DestroyEOSKitSession();
	CurrentLobbyId.Empty();
}

void UEOSKitSessionManager::UpdateEOSKitSession(const TMap<FString, FEOSKitAttribute>& NewSettings)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Interface not valid"));
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(CurrentSessionName);
	if (ExistingSession == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: No active session to update"));
		return;
	}

	// Update settings
	FOnlineSessionSettings* Settings = &ExistingSession->SessionSettings;
	for (const auto& Setting : NewSettings)
	{
		const FEOSKitAttribute& Attribute = Setting.Value;
		if (!Attribute.StringValue.IsEmpty())
		{
			Settings->Set(FName(*Attribute.Key), Attribute.StringValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
		else if (Attribute.IntValue != 0)
		{
			Settings->Set(FName(*Attribute.Key), Attribute.IntValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
		else if (Attribute.BoolValue)
		{
			Settings->Set(FName(*Attribute.Key), Attribute.BoolValue, EOnlineDataAdvertisementType::ViaOnlineService);
		}
	}

	SessionInterface->UpdateSession(CurrentSessionName, *Settings);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session updated"));
}

void UEOSKitSessionManager::StartEOSKitSession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Interface not valid"));
		return;
	}

	SessionInterface->StartSession(CurrentSessionName);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session started"));
}

void UEOSKitSessionManager::EndEOSKitSession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Interface not valid"));
		return;
	}

	SessionInterface->EndSession(CurrentSessionName);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session ended"));
}

// ========== UTILITY FUNCTIONS ==========

FString UEOSKitSessionManager::GetCurrentSessionId() const
{
	if (!SessionInterface.IsValid())
	{
		return FString();
	}

	auto Session = SessionInterface->GetNamedSession(CurrentSessionName);
	if (Session != nullptr)
	{
		return Session->GetSessionIdStr();
	}

	return FString();
}

bool UEOSKitSessionManager::IsInSession() const
{
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	return SessionInterface->GetNamedSession(CurrentSessionName) != nullptr;
}

FString UEOSKitSessionManager::GetCurrentLobbyId() const
{
	return CurrentLobbyId;
}

bool UEOSKitSessionManager::IsInLobby() const
{
	return !CurrentLobbyId.IsEmpty() && IsInSession();
}

FEOSKitAttribute UEOSKitSessionManager::MakeEOSKitAttribute_String(const FString& Key, const FString& StringValue)
{
	FEOSKitAttribute Attribute;
	Attribute.Key = Key;
	Attribute.StringValue = StringValue;
	return Attribute;
}

FEOSKitAttribute UEOSKitSessionManager::MakeEOSKitAttribute_Bool(const FString& Key, bool BoolValue)
{
	FEOSKitAttribute Attribute;
	Attribute.Key = Key;
	Attribute.BoolValue = BoolValue;
	return Attribute;
}

FEOSKitAttribute UEOSKitSessionManager::MakeEOSKitAttribute_Int(const FString& Key, int32 IntValue)
{
	FEOSKitAttribute Attribute;
	Attribute.Key = Key;
	Attribute.IntValue = IntValue;
	return Attribute;
}

void UEOSKitSessionManager::BreakSessionFindStruct(
	const FEOSKitSessionResult& SessionResult,
	FString& SessionName,
	int32& CurrentNumberOfPlayers,
	int32& MaxNumberOfPlayers,
	bool& bIsDedicatedServer,
	TMap<FString, FEOSKitAttribute>& SessionSettings)
{
	SessionName = SessionResult.SessionName;
	CurrentNumberOfPlayers = SessionResult.CurrentNumberOfPlayers;
	MaxNumberOfPlayers = SessionResult.MaxNumberOfPlayers;
	bIsDedicatedServer = SessionResult.bIsDedicatedServer;
	SessionSettings = SessionResult.SessionSettings;
}

FEOSKitCreateSessionSettings UEOSKitSessionManager::MakeCreateSessionExtraSettings(
	bool bIsLanMatch,
	int32 NumberOfPrivateConnections,
	bool bShouldAdvertise,
	bool bAllowJoinInProgress,
	EEOSKitRegion Region,
	bool bUsePresence,
	bool bAllowJoinViaPresence,
	bool bAllowJoinViaPresenceFriendsOnly,
	bool bEnforceSanctions)
{
	FEOSKitCreateSessionSettings Settings;
	Settings.bIsLanMatch = bIsLanMatch;
	Settings.NumberOfPrivateConnections = NumberOfPrivateConnections;
	Settings.bShouldAdvertise = bShouldAdvertise;
	Settings.bAllowJoinInProgress = bAllowJoinInProgress;
	Settings.Region = Region;
	Settings.bUsePresence = bUsePresence;
	Settings.bAllowJoinViaPresence = bAllowJoinViaPresence;
	Settings.bAllowJoinViaPresenceFriendsOnly = bAllowJoinViaPresenceFriendsOnly;
	Settings.bEnforceSanctions = bEnforceSanctions;
	return Settings;
}

FEOSKitCreateLobbySettings UEOSKitSessionManager::MakeCreateLobbySettings(
	bool bIsLanMatch,
	bool bAllowInvites,
	int32 NumberOfPrivateConnections,
	bool bShouldAdvertise,
	bool bAllowJoinInProgress,
	EEOSKitRegion Region,
	bool bUseVoiceChat,
	bool bUsePresence,
	const FString& BucketId,
	bool bSupportHostMigration,
	bool bEnableJoinViaId,
	const FString& LobbyIdOverride)
{
	FEOSKitCreateLobbySettings Settings;
	Settings.bIsLanMatch = bIsLanMatch;
	Settings.bAllowInvites = bAllowInvites;
	Settings.NumberOfPrivateConnections = NumberOfPrivateConnections;
	Settings.bShouldAdvertise = bShouldAdvertise;
	Settings.bAllowJoinInProgress = bAllowJoinInProgress;
	Settings.Region = Region;
	Settings.bUseVoiceChat = bUseVoiceChat;
	Settings.bUsePresence = bUsePresence;
	Settings.BucketId = BucketId;
	Settings.bSupportHostMigration = bSupportHostMigration;
	Settings.bEnableJoinViaId = bEnableJoinViaId;
	Settings.LobbyIdOverride = LobbyIdOverride;
	return Settings;
}

// ========== CALLBACK HANDLERS ==========

void UEOSKitSessionManager::OnCreateSessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session creation %s"), bWasSuccessful ? TEXT("successful") : TEXT("failed"));
	
	if (bWasSuccessful)
	{
		// Check if this was a lobby
		auto Session = SessionInterface->GetNamedSession(SessionName);
		if (Session != nullptr)
		{
			FString GameMode;
			Session->SessionSettings.Get(SETTING_GAMEMODE, GameMode);
			
			if (GameMode == TEXT("Lobby"))
			{
				CurrentLobbyId = Session->GetSessionIdStr();
				OnCreateLobbySuccess.Broadcast(CurrentLobbyId);
				UE_LOG(LogTemp, Log, TEXT("EOSKit: Lobby created with ID: %s"), *CurrentLobbyId);
				return;
			}
		}
	}
	
	OnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UEOSKitSessionManager::OnFindSessionsCompleteInternal(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session search %s"), bWasSuccessful ? TEXT("successful") : TEXT("failed"));
	
	TArray<FEOSKitSessionResult> Results;
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Found %d session(s)"), SessionSearch->SearchResults.Num());
		
		for (const auto& SearchResult : SessionSearch->SearchResults)
		{
			Results.Add(ConvertToSessionResult(SearchResult));
		}
	}
	
	OnFindSessionsComplete.Broadcast(bWasSuccessful, Results);
}

void UEOSKitSessionManager::OnJoinSessionCompleteInternal(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bWasSuccessful = (Result == EOnJoinSessionCompleteResult::Success);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session join %s"), bWasSuccessful ? TEXT("successful") : TEXT("failed"));
	
	OnJoinSessionComplete.Broadcast(bWasSuccessful);
	
	if (bWasSuccessful && SessionInterface.IsValid())
	{
		// Travel to the session
		FString ConnectInfo;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
		{
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(ConnectInfo, TRAVEL_Absolute);
			}
		}
	}
}

void UEOSKitSessionManager::OnDestroySessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session destruction %s"), bWasSuccessful ? TEXT("successful") : TEXT("failed"));
	
	if (bWasSuccessful)
	{
		CurrentLobbyId.Empty();
	}
	
	OnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UEOSKitSessionManager::OnStartSessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session start %s"), bWasSuccessful ? TEXT("successful") : TEXT("failed"));
}

void UEOSKitSessionManager::OnEndSessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Session end %s"), bWasSuccessful ? TEXT("successful") : TEXT("failed"));
}

// ========== HELPER FUNCTIONS ==========

void UEOSKitSessionManager::SetupSessionSettings(FOnlineSessionSettings& OutSettings, const FEOSKitCreateSessionSettings& InSettings)
{
	OutSettings.NumPublicConnections = InSettings.NumberOfPublicConnections;
	OutSettings.NumPrivateConnections = InSettings.NumberOfPrivateConnections;
	OutSettings.bIsLANMatch = InSettings.bIsLanMatch;
	OutSettings.bShouldAdvertise = InSettings.bShouldAdvertise;
	OutSettings.bAllowJoinInProgress = InSettings.bAllowJoinInProgress;
	OutSettings.bAllowJoinViaPresence = InSettings.bAllowJoinViaPresence;
	OutSettings.bAllowJoinViaPresenceFriendsOnly = InSettings.bAllowJoinViaPresenceFriendsOnly;
	OutSettings.bUsesPresence = InSettings.bUsePresence;
	OutSettings.bUsesStats = InSettings.bUsesStats;
	OutSettings.bIsDedicated = InSettings.bIsDedicatedServer;
	OutSettings.bUseLobbiesIfAvailable = false;
	
	// Set session name
	OutSettings.Set(SETTING_MAPNAME, InSettings.SessionName, EOnlineDataAdvertisementType::ViaOnlineService);
	OutSettings.Set(SETTING_GAMEMODE, FString("MatchmakingSession"), EOnlineDataAdvertisementType::ViaOnlineService);
	
	// Set region
	if (InSettings.Region != EEOSKitRegion::NoSelection)
	{
		OutSettings.Set(FName("REGION"), RegionToString(InSettings.Region), EOnlineDataAdvertisementType::ViaOnlineService);
	}
}

void UEOSKitSessionManager::SetupLobbySettings(FOnlineSessionSettings& OutSettings, const FEOSKitCreateLobbySettings& InSettings)
{
	OutSettings.NumPublicConnections = InSettings.NumberOfPublicConnections;
	OutSettings.NumPrivateConnections = InSettings.NumberOfPrivateConnections;
	OutSettings.bIsLANMatch = InSettings.bIsLanMatch;
	OutSettings.bShouldAdvertise = InSettings.bShouldAdvertise;
	OutSettings.bAllowJoinInProgress = InSettings.bAllowJoinInProgress;
	OutSettings.bUsesPresence = InSettings.bUsePresence;
	OutSettings.bUseLobbiesIfAvailable = true;
	OutSettings.bUseLobbiesVoiceChatIfAvailable = InSettings.bUseVoiceChat;
	
	// Set lobby name
	OutSettings.Set(SETTING_MAPNAME, InSettings.SessionName, EOnlineDataAdvertisementType::ViaOnlineService);
	
	// Set bucket ID
	if (!InSettings.BucketId.IsEmpty())
	{
		OutSettings.Set(FName("BUCKETID"), InSettings.BucketId, EOnlineDataAdvertisementType::ViaOnlineService);
	}
	
	// Set region
	if (InSettings.Region != EEOSKitRegion::NoSelection)
	{
		OutSettings.Set(FName("REGION"), RegionToString(InSettings.Region), EOnlineDataAdvertisementType::ViaOnlineService);
	}
}

FString UEOSKitSessionManager::RegionToString(EEOSKitRegion Region) const
{
	switch (Region)
	{
	case EEOSKitRegion::NAEast: return TEXT("NAEast");
	case EEOSKitRegion::NAWest: return TEXT("NAWest");
	case EEOSKitRegion::Europe: return TEXT("Europe");
	case EEOSKitRegion::Asia: return TEXT("Asia");
	case EEOSKitRegion::Oceania: return TEXT("Oceania");
	case EEOSKitRegion::SouthAmerica: return TEXT("SouthAmerica");
	default: return TEXT("");
	}
}

FEOSKitSessionResult UEOSKitSessionManager::ConvertToSessionResult(const FOnlineSessionSearchResult& SearchResult)
{
	FEOSKitSessionResult Result;
	
	Result.SessionId = SearchResult.GetSessionIdStr();
	Result.CurrentNumberOfPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
	Result.MaxNumberOfPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
	Result.bIsDedicatedServer = SearchResult.Session.SessionSettings.bIsDedicated;
	Result.Ping = SearchResult.PingInMs;
	Result.bIsLanMatch = SearchResult.Session.SessionSettings.bIsLANMatch;
	Result.bAllowJoinInProgress = SearchResult.Session.SessionSettings.bAllowJoinInProgress;
	
	// Get session name
	FString SessionName;
	SearchResult.Session.SessionSettings.Get(SETTING_MAPNAME, SessionName);
	Result.SessionName = SessionName;
	
	// Get owner name
	if (SearchResult.Session.OwningUserName.IsEmpty())
	{
		Result.OwnerName = SearchResult.Session.OwningUserId->ToString();
	}
	else
	{
		Result.OwnerName = SearchResult.Session.OwningUserName;
	}
	
	// Extract all custom settings
	for (const auto& Setting : SearchResult.Session.SessionSettings.Settings)
	{
		FEOSKitAttribute Attribute;
		Attribute.Key = Setting.Key.ToString();
		
		if (Setting.Value.Data.GetType() == EOnlineKeyValuePairDataType::String)
		{
			Setting.Value.Data.GetValue(Attribute.StringValue);
		}
		else if (Setting.Value.Data.GetType() == EOnlineKeyValuePairDataType::Int32)
		{
			Setting.Value.Data.GetValue(Attribute.IntValue);
		}
		else if (Setting.Value.Data.GetType() == EOnlineKeyValuePairDataType::Bool)
		{
			Setting.Value.Data.GetValue(Attribute.BoolValue);
		}
		
		Result.SessionSettings.Add(Attribute.Key, Attribute);
	}
	
	return Result;
}
