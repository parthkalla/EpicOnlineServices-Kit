#include "EOSKitSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Runtime/Core/Public/Misc/CommandLine.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Interfaces/OnlineTitleFileInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
// Try to include OnlineStoreInterfaceV2 - UE 5.5 uses OnlineStoreInterfaceV2.h
// In UE 5.5, this interface may have been moved or renamed
#if __has_include("Interfaces/OnlineStoreInterfaceV2.h")
#include "Interfaces/OnlineStoreInterfaceV2.h"
#elif __has_include("Interfaces/OnlineStoreV2Interface.h")
#include "Interfaces/OnlineStoreV2Interface.h"
#else
// Forward declare if header not available
class IOnlineStoreV2;
typedef TSharedPtr<class IOnlineStoreV2, ESPMode::ThreadSafe> IOnlineStoreV2Ptr;
#endif
#include "Interfaces/OnlinePurchaseInterface.h"
#if __has_include("OnlineSubsystemEOS.h")
#include "OnlineSubsystemEOS.h"
#endif
#include "IEOSSDKManager.h"
#if WITH_EOS_SDK
#include "eos_types.h"
#include "eos_common.h"
#include "eos_sdk.h"
#include "eos_connect.h"
#endif

UEOSKitSubsystem::UEOSKitSubsystem()
	: ReadRef(MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>())
{
	// Add the delegate to the online subsystem
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			// Note: Session invite accepted delegate would need a different handler
			// For now, we'll handle it in JoinSession
		}
	}
}

void UEOSKitSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Subsystem Deinitializing - cleaning up sessions..."));
	
	// Destroy all active sessions when subsystem is destroyed (host leaving/disconnecting)
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			// Check all common session names (IOnlineSession doesn't have GetAllSessions)
			TArray<FName> SessionNamesToCheck = { 
				NAME_GameSession, 
				NAME_PartySession,
				FName(TEXT("Modified_EOS_Session")),
				FName(TEXT("Modified_EOS_Lobby")),
				FName(TEXT("test")),
				FName(TEXT("Sessions")),
				FName(TEXT("Session")),
				FName(TEXT("Lobby"))
			};
			
			int32 DestroyedCount = 0;
			
			for (const FName& SessionName : SessionNamesToCheck)
			{
				if (FNamedOnlineSession* Session = SessionPtrRef->GetNamedSession(SessionName))
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Destroying session '%s' on subsystem cleanup"), *SessionName.ToString());
					
					// End session first if it's started
					if (Session->SessionState == EOnlineSessionState::InProgress)
					{
						SessionPtrRef->EndSession(SessionName);
					}
					
					// Destroy the session
					SessionPtrRef->DestroySession(SessionName);
					DestroyedCount++;
				}
			}
			
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Destroyed %d session(s) on deinitialize"), DestroyedCount);
		}
	}
	
	Super::Deinitialize();
}

// ========================================
// Login Functions
// ========================================

void UEOSKitSubsystem::Login(int32 LocalUserNum, const FString& ID, const FString& Token, const FString& Type, const FBP_EOSKit_Login_Callback& Result)
{
	LoginCallBackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			FOnlineAccountCredentials AccountDetails;
			AccountDetails.Id = ID;
			AccountDetails.Token = Token;
			AccountDetails.Type = Type;
			IdentityPointerRef->OnLoginCompleteDelegates->AddUObject(this, &UEOSKitSubsystem::LoginCallback);
			IdentityPointerRef->Login(LocalUserNum, AccountDetails);
		}
		else
		{
			Result.ExecuteIfBound(false, TEXT("Failed to get Identity Pointer"));
		}
	}
	else
	{
		Result.ExecuteIfBound(false, TEXT("Failed to get Subsystem"));
	}
}

void UEOSKitSubsystem::LoginWithDeviceID(int32 LocalUserNum, const FString& DisplayName, const FString& DeviceName, const FBP_EOSKit_Login_Callback& Result)
{
	Login(LocalUserNum, DisplayName, DeviceName, TEXT("deviceid"), Result);
}

void UEOSKitSubsystem::LoginWithAccountPortal(int32 LocalUserNum, const FBP_EOSKit_Login_Callback& Result)
{
	Login(LocalUserNum, TEXT(""), TEXT(""), TEXT("accountportal"), Result);
}

void UEOSKitSubsystem::LoginWithSteam(int32 LocalUserNum, const FBP_EOSKit_Login_Callback& Result)
{
	Login(LocalUserNum, TEXT(""), TEXT(""), TEXT("steam"), Result);
}

void UEOSKitSubsystem::LoginWithPersistantAuth(int32 LocalUserNum, const FBP_EOSKit_Login_Callback& Result)
{
	Login(LocalUserNum, TEXT(""), TEXT(""), TEXT("persistentauth"), Result);
}

void UEOSKitSubsystem::LoginWithDeveloperTool(int32 LocalUserNum, const FString& LocalIP, const FString& Credential, const FBP_EOSKit_Login_Callback& Result)
{
	Login(LocalUserNum, LocalIP, Credential, TEXT("developer"), Result);
}

void UEOSKitSubsystem::LoginWithEpicLauncher(int32 LocalUserNum, const FBP_EOSKit_Login_Callback& Result)
{
	FString EGS_Token;
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_PASSWORD="), EGS_Token);
	Login(LocalUserNum, TEXT(""), EGS_Token, TEXT("exchangecode"), Result);
}

void UEOSKitSubsystem::Logout(int32 LocalUserNum, const FBP_EOSKit_Logout_Callback& Result)
{
	LogoutCallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			IdentityPointerRef->OnLogoutCompleteDelegates->AddUObject(this, &UEOSKitSubsystem::LogoutCallback);
			IdentityPointerRef->Logout(LocalUserNum);
		}
		else
		{
			Result.ExecuteIfBound(false);
		}
	}
	else
	{
		Result.ExecuteIfBound(false);
	}
}

// ========================================
// User Information
// ========================================

FString UEOSKitSubsystem::GetPlayerNickname(const int32 LocalUserNum)
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			return IdentityPointerRef->GetPlayerNickname(LocalUserNum);
		}
		return FString();
	}
	return FString();
}

bool UEOSKitSubsystem::GetLoginStatus(const int32 LocalUserNum)
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (IdentityPointerRef->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// ========================================
// Session Functions
// ========================================

void UEOSKitSubsystem::CreateEOSSession(
	const FBP_EOSKit_CreateSession_Callback& Result,
	TMap<FString, FString> Custom_Settings,
	FString SessionName,
	bool bIsDedicatedServer,
	bool bIsLan,
	int32 NumberOfPublicConnections,
	EEOSKitRegion Region)
{
	CreateSession_CallbackBP = Result;
	if (IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld()))
	{
		if (IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			// Check if a session with this name already exists
			FName SessionFName = FName(*SessionName);
			if (FNamedOnlineSession* ExistingSession = SessionPtrRef->GetNamedSession(SessionFName))
			{
				UE_LOG(LogTemp, Error, TEXT("EOSKit: Session '%s' already exists! Please destroy it first or use a different name."), *SessionName);
				Result.ExecuteIfBound(false, SessionFName);
				return;
			}

			FOnlineSessionSettings SessionCreationInfo;
			SessionCreationInfo.bIsDedicated = bIsDedicatedServer;
			SessionCreationInfo.bUsesPresence = true;
			SessionCreationInfo.bAllowJoinViaPresence = true;
			SessionCreationInfo.bAllowJoinViaPresenceFriendsOnly = false;
			SessionCreationInfo.bAllowInvites = true;
			if (bIsDedicatedServer)
			{
				SessionCreationInfo.bUsesPresence = false;
				SessionCreationInfo.bAllowJoinViaPresence = false;
				SessionCreationInfo.bAllowJoinViaPresenceFriendsOnly = false;
				SessionCreationInfo.bAllowInvites = false;
			}
			SessionCreationInfo.bIsLANMatch = bIsLan;
			SessionCreationInfo.NumPublicConnections = NumberOfPublicConnections;
			SessionCreationInfo.bUseLobbiesIfAvailable = false;
			SessionCreationInfo.bUseLobbiesVoiceChatIfAvailable = false;
			SessionCreationInfo.bShouldAdvertise = true;
			SessionCreationInfo.bAllowJoinInProgress = true;

			SessionCreationInfo.Settings.Add(FName(TEXT("REGIONINFO")), FOnlineSessionSetting(UEnum::GetValueAsString(Region), EOnlineDataAdvertisementType::ViaOnlineService));
			if (bIsDedicatedServer)
			{
				SessionCreationInfo.Settings.Add(FName(TEXT("PortInfo")), FOnlineSessionSetting(GetWorld()->URL.Port, EOnlineDataAdvertisementType::ViaOnlineService));
			}
			SessionCreationInfo.Set(SEARCH_KEYWORDS, FString(SessionName), EOnlineDataAdvertisementType::ViaOnlineService);
			for (auto& Settings_SingleValue : Custom_Settings)
			{
				if (Settings_SingleValue.Key.Len() == 0)
				{
					continue;
				}

				FOnlineSessionSetting Setting;
				Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				Setting.Data.SetValue(Settings_SingleValue.Value);
				SessionCreationInfo.Set(FName(*Settings_SingleValue.Key), Setting);
			}
			SessionPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnCreateSessionCompleted);
			SessionPtrRef->CreateSession(0, *SessionName, SessionCreationInfo);
		}
	}
}

void UEOSKitSubsystem::CreateEOSLobby(
	const FBP_EOSKit_CreateLobby_Callback& Result,
	TMap<FString, FString> Custom_Settings,
	FString SessionName,
	bool bUseVoiceChat,
	bool bUsePresence,
	bool bAllowInvites,
	bool bAdvertise,
	bool bAllowJoinInProgress,
	bool bIsLan,
	int32 NumberOfPublicConnections,
	int32 NumberOfPrivateConnections)
{
	CreateLobby_CallbackBP = Result;
	if (IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld()))
	{
		if (IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			// Check if a lobby with this name already exists
			FName SessionFName = FName(*SessionName);
			if (FNamedOnlineSession* ExistingSession = SessionPtrRef->GetNamedSession(SessionFName))
			{
				UE_LOG(LogTemp, Error, TEXT("EOSKit: Lobby '%s' already exists! Please destroy it first or use a different name."), *SessionName);
				Result.ExecuteIfBound(false, SessionFName);
				return;
			}

			FOnlineSessionSettings SessionCreationInfo;
			SessionCreationInfo.bIsDedicated = false;
			SessionCreationInfo.bAllowInvites = bAllowInvites;
			SessionCreationInfo.bIsLANMatch = bIsLan;
			SessionCreationInfo.NumPublicConnections = NumberOfPublicConnections;
			SessionCreationInfo.NumPrivateConnections = NumberOfPrivateConnections;
			SessionCreationInfo.bUseLobbiesIfAvailable = true;
			SessionCreationInfo.bUseLobbiesVoiceChatIfAvailable = bUseVoiceChat;
			SessionCreationInfo.bUsesPresence = bUsePresence;
			SessionCreationInfo.bAllowJoinViaPresence = bUsePresence;
			SessionCreationInfo.bAllowJoinViaPresenceFriendsOnly = bUsePresence;
			SessionCreationInfo.bShouldAdvertise = bAdvertise;
			SessionCreationInfo.bAllowJoinInProgress = bAllowJoinInProgress;

			SessionCreationInfo.Set(SEARCH_KEYWORDS, FString(SessionName), EOnlineDataAdvertisementType::ViaOnlineService);
			for (auto& Settings_SingleValue : Custom_Settings)
			{
				if (Settings_SingleValue.Key.Len() == 0)
				{
					continue;
				}
				FOnlineSessionSetting Setting;
				Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				Setting.Data.SetValue(Settings_SingleValue.Value);
				SessionCreationInfo.Set(FName(*Settings_SingleValue.Key), Setting);
			}
			SessionPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnCreateLobbyCompleted);
			SessionPtrRef->CreateSession(0, *SessionName, SessionCreationInfo);
		}
	}
}

void UEOSKitSubsystem::FindEOSSession(
	const FBP_EOSKit_FindSession_Callback& Result,
	TMap<FString, FString> Search_Settings,
	EEOSKitMatchType MatchType,
	EEOSKitRegion RegionToSearch)
{
	FindSession_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->QuerySettings.SearchParams.Empty();
			SessionSearch->bIsLanQuery = false;
			if (MatchType == EEOSKitMatchType::MatchmakingSession)
			{
				if (RegionToSearch != EEOSKitRegion::NoSelection)
				{
					SessionSearch->QuerySettings.Set(FName(TEXT("RegionInfo")), UEnum::GetValueAsString(RegionToSearch), EOnlineComparisonOp::Equals);
				}
			}
			else
			{
				SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
			}
			if (Search_Settings.Num() > 0)
			{
				for (auto& Settings_SingleValue : Search_Settings)
				{
					if (Settings_SingleValue.Key.Len() == 0)
					{
						continue;
					}
					FOnlineSessionSetting Setting;
					Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
					Setting.Data.SetValue(Settings_SingleValue.Value);
					SessionSearch->QuerySettings.Set(FName(*Settings_SingleValue.Key), Settings_SingleValue.Value, EOnlineComparisonOp::Equals);
				}
			}
			SessionSearch->MaxSearchResults = 1000;
			SessionPtrRef->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnFindSessionCompleted);
			SessionPtrRef->FindSessions(0, SessionSearch.ToSharedRef());
		}
		else
		{
			Result.ExecuteIfBound(false, TArray<FEOSKitSessionFindStruct>());
		}
	}
	else
	{
		Result.ExecuteIfBound(false, TArray<FEOSKitSessionFindStruct>());
	}
}

void UEOSKitSubsystem::DestroyEosSession(const FBP_EOSKit_DestroySession_Callback& Result, FName SessionName)
{
	DestroySession_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			// Check if session exists before trying to destroy it
			if (FNamedOnlineSession* Session = SessionPtrRef->GetNamedSession(SessionName))
			{
				// End the session first if it's started
				if (Session->SessionState == EOnlineSessionState::InProgress)
				{
					SessionPtrRef->EndSession(SessionName);
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Ended session '%s' before destroying"), *SessionName.ToString());
				}

				SessionPtrRef->OnDestroySessionCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnDestroySessionCompleted);
				SessionPtrRef->DestroySession(SessionName);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session '%s' does not exist, cannot destroy"), *SessionName.ToString());
				Result.ExecuteIfBound(false);
			}
		}
		else
		{
			Result.ExecuteIfBound(false);
		}
	}
	else
	{
		Result.ExecuteIfBound(false);
	}
}

void UEOSKitSubsystem::JoinEosSession(
	const FBP_EOSKit_JoinSession_Callback& Result,
	FName SessionName,
	bool bIsDedicatedServerSession,
	FBlueprintSessionResult SessionResult)
{
	Local_bIsDedicatedServerSession = bIsDedicatedServerSession;
	JoinSession_CallbackBP = Result;
	const IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());
	if (SessionResult.OnlineResult.IsSessionInfoValid())
	{
		if (SubsystemRef)
		{
			if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
			{
				if (SessionResult.OnlineResult.Session.SessionSettings.Settings.Num() > 0)
				{
				}
				else
				{
					Result.ExecuteIfBound(false);
				}
				SessionPtrRef->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnJoinSessionCompleted);
				SessionPtrRef->JoinSession(0, SessionName, SessionResult.OnlineResult);
			}
			else
			{
				Result.ExecuteIfBound(false);
			}
		}
		else
		{
			Result.ExecuteIfBound(false);
		}
	}
	else
	{
		Result.ExecuteIfBound(false);
	}
}

// ========================================
// Session Management
// ========================================

void UEOSKitSubsystem::UnRegisterPlayer(FName SessionName)
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				SessionPtrRef->UnregisterPlayer(SessionName, *IdentityPointerRef->GetUniquePlayerId(0));
			}
		}
	}
}

void UEOSKitSubsystem::RegisterPlayer(FName SessionName, bool bWasInvited)
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				SessionPtrRef->RegisterPlayer(SessionName, *IdentityPointerRef->GetUniquePlayerId(0), bWasInvited);
			}
		}
	}
}

void UEOSKitSubsystem::StartSession(FName SessionName)
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			SessionPtrRef->StartSession(SessionName);
		}
	}
}

void UEOSKitSubsystem::EndSession(FName SessionName)
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			SessionPtrRef->EndSession(SessionName);
		}
	}
}

// ========================================
// Social Features
// ========================================

bool UEOSKitSubsystem::ShowFriendUserInterface()
{
	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem != nullptr)
	{
		const IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
		if (ExternalUI.IsValid())
		{
			return ExternalUI->ShowFriendsUI(0);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// ========================================
// Statistics
// ========================================

void UEOSKitSubsystem::UpdateStats(const FBP_EOSKit_UpdateStat_Callback& Result, const FString& StatName, int32 Amount)
{
	UpdateStat_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineStatsPtr StatsPointerRef = SubsystemRef->GetStatsInterface())
			{
				FOnlineStatsUserUpdatedStats StatVar = FOnlineStatsUserUpdatedStats(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef());
				StatVar.Stats.Add(StatName, FOnlineStatUpdate(Amount, FOnlineStatUpdate::EOnlineStatModificationType::Sum));
				TArray<FOnlineStatsUserUpdatedStats> StatArray;
				StatArray.Add(StatVar);
				StatsPointerRef->UpdateStats(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef(), StatArray, FOnlineStatsUpdateStatsComplete::CreateUObject(this, &UEOSKitSubsystem::OnUpdateStatsCompleted));
			}
		}
	}
}

void UEOSKitSubsystem::GetStats(const FBP_EOSKit_GetStats_Callback& Result, const TArray<FString>& StatName)
{
	GetStats_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineStatsPtr StatsPointerRef = SubsystemRef->GetStatsInterface())
			{
				TArray<TSharedRef<const FUniqueNetId>> Usersvar;
				Usersvar.Add(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef());
				StatsPointerRef->QueryStats(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef(), Usersvar, StatName, FOnlineStatsQueryUsersStatsComplete::CreateUObject(this, &UEOSKitSubsystem::OnGetStatsCompleted));
			}
		}
	}
}

// ========================================
// Player Data
// ========================================

void UEOSKitSubsystem::SetPlayerData(const FBP_EOSKit_WriteFile_Callback& Result, const FString& FileName, USaveGame* SavedGame)
{
	WriteFile_CallbackBP = Result;
	if (SavedGame)
	{
		TArray<uint8> LocalArray;
		UGameplayStatics::SaveGameToMemory(SavedGame, LocalArray);
		if (LocalArray.Num() > 0)
		{
			if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
			{
				if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
				{
					if (const IOnlineUserCloudPtr CloudPointerRef = SubsystemRef->GetUserCloudInterface())
					{
						const TSharedPtr<const FUniqueNetId> UserIDRef = IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef();
						CloudPointerRef->OnWriteUserFileCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnWriteFileComplete);
						CloudPointerRef->WriteUserFile(*UserIDRef, FileName, LocalArray);
					}
					else
					{
						WriteFile_CallbackBP.ExecuteIfBound(false);
					}
				}
				else
				{
					WriteFile_CallbackBP.ExecuteIfBound(false);
				}
			}
			else
			{
				WriteFile_CallbackBP.ExecuteIfBound(false);
			}
		}
		else
		{
			WriteFile_CallbackBP.ExecuteIfBound(false);
		}
	}
	else
	{
		WriteFile_CallbackBP.ExecuteIfBound(false);
	}
}

void UEOSKitSubsystem::GetPlayerData(const FBP_EOSKit_GetFile_Callback& Result, const FString& FileName)
{
	GetFile_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineUserCloudPtr CloudPointerRef = SubsystemRef->GetUserCloudInterface())
			{
				TSharedPtr<const FUniqueNetId> UserIDRef = IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef();
				CloudPointerRef->OnReadUserFileCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnGetFileComplete);
				CloudPointerRef->ReadUserFile(*UserIDRef, FileName);
			}
			else
			{
				GetFile_CallbackBP.ExecuteIfBound(false, nullptr);
			}
		}
		else
		{
			GetFile_CallbackBP.ExecuteIfBound(false, nullptr);
		}
	}
	else
	{
		GetFile_CallbackBP.ExecuteIfBound(false, nullptr);
	}
}

// ========================================
// Title Files
// ========================================

void UEOSKitSubsystem::EnumerateTitleFiles(const FBP_EOSKit_TitleFileList_Callback& Result)
{
	TitleFileList_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineTitleFilePtr TitleFilePtr = SubsystemRef->GetTitleFileInterface())
			{
				TitleFilePtr->OnEnumerateFilesCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnTitleFileListComplete);
				TitleFilePtr->EnumerateFiles();
			}
			else
			{
				TitleFileList_CallbackBP.ExecuteIfBound(false, TEXT("Failed to get Title File Interface"));
			}
		}
		else
		{
			TitleFileList_CallbackBP.ExecuteIfBound(false, TEXT("Failed to get Online Identity"));
		}
	}
	else
	{
		TitleFileList_CallbackBP.ExecuteIfBound(false, TEXT("Failed to get Online Subsystem"));
	}
}

TArray<FFileListStruct> UEOSKitSubsystem::GetTitleFileList()
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineTitleFilePtr TitleFilePtr = SubsystemRef->GetTitleFileInterface())
			{
				TArray<FCloudFileHeader> Files;
				TitleFilePtr->GetFileList(Files);
				TArray<FFileListStruct> Local_FileList;
				for (int i = 0; i < Files.Num(); i++)
				{
					FFileListStruct Temp;
					Temp.FileName = Files[i].FileName;
					Temp.FileSize = Files[i].FileSize;
					Temp.Hash = Files[i].Hash;
					Temp.HashType = Files[i].HashType;
					Temp.iChunkID = Files[i].ChunkID;
					Temp.DLName = Files[i].DLName;
					Temp.ExternalStorageIds = Files[i].ExternalStorageIds;
					Temp.URL = Files[i].URL;
					Local_FileList.Add(Temp);
				}
				return Local_FileList;
			}
		}
	}
	return TArray<FFileListStruct>();
}

void UEOSKitSubsystem::GetTitleFile(const FBP_EOSKit_GetTitleFile_Callback& Result, const FString& FileName)
{
	GetTitleFile_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineTitleFilePtr TitleFilePtr = SubsystemRef->GetTitleFileInterface())
			{
				TitleFilePtr->OnReadFileCompleteDelegates.AddUObject(this, &UEOSKitSubsystem::OnTitleFileComplete);
				TitleFilePtr->ReadFile(FileName);
		return;
			}
		}
	}
	GetTitleFile_CallbackBP.ExecuteIfBound(false);
}

TArray<uint8> UEOSKitSubsystem::GetTitleFileContent(const FString& FileName)
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineTitleFilePtr TitleFilePtr = SubsystemRef->GetTitleFileInterface())
			{
				TArray<uint8> TitleFileContent;
				TitleFilePtr->GetFileContents(FileName, TitleFileContent);
				return TitleFileContent;
			}
		}
	}
	return TArray<uint8>();
}

// ========================================
// Leaderboard
// ========================================

void UEOSKitSubsystem::GetLeaderboard(const FBP_EOSKit_GetFile_Callback& Result, FName LeaderboardName, int32 Rank, int32 Range)
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (IOnlineIdentityPtr Identity = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineLeaderboardsPtr Leaderboards = SubsystemRef->GetLeaderboardsInterface())
			{
				ReadRef = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
				FString LeaderboardNameString = LeaderboardName.ToString();
				ReadRef->LeaderboardName = LeaderboardNameString;
#else
				ReadRef->LeaderboardName = LeaderboardName;
#endif
				Leaderboards->AddOnLeaderboardReadCompleteDelegate_Handle(FOnLeaderboardReadComplete::FDelegate::CreateUObject(this, &UEOSKitSubsystem::OnLeaderboardListCompleted));
				Leaderboards->ReadLeaderboardsAroundRank(Rank, Range, ReadRef);
			}
		}
	}
}

// ========================================
// Store
// ========================================

void UEOSKitSubsystem::PurchaseItem(const FBP_EOSKit_PurchaseOffer_Callback& Result, const FString& ItemID)
{
	PurchaseOffer_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld()))
	{
		if (const IOnlineStoreV2Ptr StoreV2Ptr = SubsystemRef->GetStoreV2Interface())
		{
			if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				if (const IOnlinePurchasePtr Purchase = SubsystemRef->GetPurchaseInterface())
				{
					FPurchaseCheckoutRequest Request = {};
					Request.AddPurchaseOffer(TEXT(""), ItemID, 1);

					Purchase->Checkout(*IdentityPointerRef->GetUniquePlayerId(0).Get(),
						Request,
						FOnPurchaseCheckoutComplete::CreateLambda(
							[this](
							const FOnlineError& Result,
							const TSharedRef<FPurchaseReceipt>& Receipt)
							{
								if (Result.WasSuccessful())
								{
									PurchaseOffer_CallbackBP.ExecuteIfBound(true);
								}
								else
								{
									PurchaseOffer_CallbackBP.ExecuteIfBound(false);
								}
							})
					);
				}
				else
				{
					PurchaseOffer_CallbackBP.ExecuteIfBound(false);
				}
			}
			else
			{
				PurchaseOffer_CallbackBP.ExecuteIfBound(false);
			}
		}
		else
		{
			PurchaseOffer_CallbackBP.ExecuteIfBound(false);
		}
	}
	else
	{
		PurchaseOffer_CallbackBP.ExecuteIfBound(false);
	}
}

void UEOSKitSubsystem::QueryOffers(const FBP_EOSKit_GetOffers_Callback& Result)
{
	GetOffers_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld()))
	{
		if (const IOnlineStoreV2Ptr StoreV2Ptr = SubsystemRef->GetStoreV2Interface())
		{
			if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				StoreV2Ptr->QueryOffersByFilter(*IdentityPointerRef->GetUniquePlayerId(0).Get(), FOnlineStoreFilter(),
					FOnQueryOnlineStoreOffersComplete::CreateLambda([
						StoreV2Wk = TWeakPtr<IOnlineStoreV2, ESPMode::ThreadSafe>(StoreV2Ptr), this](
						bool bWasSuccessful,
						const TArray<FUniqueOfferId>& OfferIds,
						const FString& Error)
						{
							if (const auto StoreV2 = StoreV2Wk.Pin())
							{
								if (bWasSuccessful && StoreV2.IsValid())
								{
									TArray<FOnlineStoreOfferRef> Offers;
									StoreV2->GetOffers(Offers);
									TArray<FOffersStruct> OfferArray;
									for (int32 i = 0; i < Offers.Num(); ++i)
									{
										FOffersStruct Offer;
										Offer.ItemID = Offers[i]->OfferId;
										Offer.ItemName = Offers[i]->Title;
										Offer.Description = Offers[i]->Description;
										Offer.ExpirationDate = Offers[i]->ExpirationDate;
										Offer.LongDescription = Offers[i]->LongDescription;
										Offer.NumericPrice = Offers[i]->NumericPrice;
										Offer.PriceText = Offers[i]->PriceText;
										Offer.RegularPrice = Offers[i]->RegularPrice;
										Offer.ReleaseDate = Offers[i]->ReleaseDate;
										Offer.RegularPriceText = Offers[i]->RegularPriceText;
										OfferArray.Add(Offer);
									}
									GetOffers_CallbackBP.ExecuteIfBound(true, OfferArray);
								}
								else
								{
									GetOffers_CallbackBP.ExecuteIfBound(false, TArray<FOffersStruct>());
								}
							}
							else
							{
								GetOffers_CallbackBP.ExecuteIfBound(false, TArray<FOffersStruct>());
							}
						}));
			}
			else
			{
				GetOffers_CallbackBP.ExecuteIfBound(false, TArray<FOffersStruct>());
			}
		}
		else
		{
			GetOffers_CallbackBP.ExecuteIfBound(false, TArray<FOffersStruct>());
		}
	}
	else
	{
		GetOffers_CallbackBP.ExecuteIfBound(false, TArray<FOffersStruct>());
	}
}

void UEOSKitSubsystem::GetOwnedItems(const FBP_EOSKit_GetOwnedItems_Callback& Result)
{
	GetOwnedItems_CallbackBP = Result;
	if (const IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld()))
	{
		if (const IOnlineStoreV2Ptr StoreV2Ptr = SubsystemRef->GetStoreV2Interface())
		{
			if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				if (const IOnlinePurchasePtr Purchase = SubsystemRef->GetPurchaseInterface())
				{
					Purchase->QueryReceipts(*IdentityPointerRef->GetUniquePlayerId(0).Get(), false,
						FOnQueryReceiptsComplete::CreateLambda(
							[this, SubsystemRef, IdentityPointerRef, Purchase](const FOnlineError& Error)
							{
								if (Error.WasSuccessful())
								{
									if (Purchase)
									{
										TArray<FString> ItemNames;
										TArray<FPurchaseReceipt> Receipts;
										Purchase->GetReceipts(*IdentityPointerRef->GetUniquePlayerId(0).Get(), Receipts);
										for (int i = 0; i < Receipts.Num(); i++)
										{
											ItemNames.Add(Receipts[i].ReceiptOffers[0].LineItems[0].ItemName);
										}
										GetOwnedItems_CallbackBP.ExecuteIfBound(true, ItemNames);
									}
									else
									{
										GetOwnedItems_CallbackBP.ExecuteIfBound(false, TArray<FString>());
									}
								}
								else
								{
									GetOwnedItems_CallbackBP.ExecuteIfBound(false, TArray<FString>());
								}
							}));
				}
				else
				{
					GetOwnedItems_CallbackBP.ExecuteIfBound(false, TArray<FString>());
				}
			}
			else
			{
				GetOwnedItems_CallbackBP.ExecuteIfBound(false, TArray<FString>());
			}
		}
		else
		{
			GetOwnedItems_CallbackBP.ExecuteIfBound(false, TArray<FString>());
		}
	}
	else
	{
		GetOwnedItems_CallbackBP.ExecuteIfBound(false, TArray<FString>());
	}
}

// ========================================
// Utility
// ========================================

FString UEOSKitSubsystem::GenerateSessionCode(int32 CodeLength) const
{
	FString SessionCode;

	for (int32 i = 0; i < CodeLength; i++)
	{
		const int32 RandomNumber = FMath::RandRange(0, 35);

		// Convert the random number into a character (0-9, A-Z)
		TCHAR RandomChar = (RandomNumber < 10) ? TCHAR('0' + RandomNumber) : TCHAR('A' + (RandomNumber - 10));

		// Append the character to the session code
		SessionCode.AppendChar(RandomChar);
	}

	return SessionCode;
}

// ========================================
// Callback Functions
// ========================================

void UEOSKitSubsystem::LoginCallback(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error) const
{
	LoginCallBackBP.ExecuteIfBound(bWasSuccess, Error);
}

void UEOSKitSubsystem::LogoutCallback(int32 LocalUserNum, bool bWasSuccess) const
{
	// Destroy all sessions when logging out (like EIK pattern)
	if (bWasSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Logout successful - destroying all sessions"));
		
		if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
		{
			if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
			{
				// Check all common session names (IOnlineSession doesn't have GetAllSessions)
				TArray<FName> SessionNamesToCheck = { 
					NAME_GameSession, 
					NAME_PartySession,
					FName(TEXT("Modified_EOS_Session")),
					FName(TEXT("Modified_EOS_Lobby")),
					FName(TEXT("test")),
					FName(TEXT("Sessions")),
					FName(TEXT("Session")),
					FName(TEXT("Lobby"))
				};
				
				int32 DestroyedCount = 0;
				
				for (const FName& SessionName : SessionNamesToCheck)
				{
					if (FNamedOnlineSession* Session = SessionPtrRef->GetNamedSession(SessionName))
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Destroying session '%s' on logout"), *SessionName.ToString());
						
						// End session first if it's started
						if (Session->SessionState == EOnlineSessionState::InProgress)
						{
							SessionPtrRef->EndSession(SessionName);
						}
						
						// Destroy the session
						SessionPtrRef->DestroySession(SessionName);
						DestroyedCount++;
					}
				}
				
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Destroyed %d session(s) on logout"), DestroyedCount);
			}
		}
	}
	
	LogoutCallbackBP.ExecuteIfBound(bWasSuccess);
}

void UEOSKitSubsystem::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful) const
{
	// Note: This is only called when using the OnlineSubsystem interface directly
	// Most users use the SDK async nodes which bypass this
	CreateSession_CallbackBP.ExecuteIfBound(bWasSuccessful, SessionName);
}

void UEOSKitSubsystem::OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful) const
{
	if (bWasSuccessful)
	{
		// EIK DOES auto-register for lobbies (line 991 in EIK_Subsystem.cpp)
		if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
		{
			if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
			{
				if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
				{
					if (TSharedPtr<const FUniqueNetId> UniqueId = IdentityPointerRef->GetUniquePlayerId(0))
					{
						bool bRegistered = SessionPtrRef->RegisterPlayer(SessionName, *UniqueId, false);
						UE_LOG(LogTemp, Log, TEXT("EOSKit: Auto-registered host in lobby '%s': %s"), 
							*SessionName.ToString(), bRegistered ? TEXT("✅ Success") : TEXT("❌ Failed"));
					}
				}
			}
		}
		CreateLobby_CallbackBP.ExecuteIfBound(bWasSuccessful, SessionName);
	}
	else
	{
		CreateLobby_CallbackBP.ExecuteIfBound(false, SessionName);
	}
}

void UEOSKitSubsystem::OnFindSessionCompleted(bool bWasSuccess) const
{
	if (const IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		TArray<FEOSKitSessionFindStruct> SessionResult_Array;
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			if (SessionSearch->SearchResults.Num() > 0)
			{
				for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
				{
					FBlueprintSessionResult SessionResult;
					SessionResult.OnlineResult = SessionSearch->SearchResults[SearchIdx];
					FOnlineSessionSettings SessionSettings = SessionResult.OnlineResult.Session.SessionSettings;
					TMap<FName, FString> AllSettingsWithData;
					TMap<FName, FOnlineSessionSetting>::TIterator It(SessionSettings.Settings);

					TMap<FString, FEOSKitAttributeData> LocalArraySettings;
					while (It)
					{
						const FName& SettingName = It.Key();
						const FOnlineSessionSetting& Setting = It.Value();
						FString SettingValueString = Setting.Data.ToString();
						LocalArraySettings.Add(*SettingName.ToString(), FEOSKitAttributeData(Setting.Data));
						++It;
					}
					FEOSKitSessionFindStruct LocalStruct;
					LocalStruct.SessionName = TEXT("GameSession");
					LocalStruct.CurrentNumberOfPlayers = (SessionResult.OnlineResult.Session.SessionSettings.NumPublicConnections + SessionResult.OnlineResult.Session.SessionSettings.NumPrivateConnections) - (SessionResult.OnlineResult.Session.NumOpenPublicConnections + SessionResult.OnlineResult.Session.NumOpenPrivateConnections);
					LocalStruct.MaxNumberOfPlayers = SessionResult.OnlineResult.Session.SessionSettings.NumPublicConnections + SessionResult.OnlineResult.Session.SessionSettings.NumPrivateConnections;
					LocalStruct.SessionResult = SessionResult;
					LocalStruct.SessionSettings = LocalArraySettings;
					SessionResult_Array.Add(LocalStruct);
				}
			}
		}

		FindSession_CallbackBP.ExecuteIfBound(bWasSuccess, SessionResult_Array);
	}
}

void UEOSKitSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// EIK does NOT auto-register on join - users must call RegisterPlayer manually
		if (APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
			{
				if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
				{
					FString JoinAddress;
					SessionPtrRef->GetResolvedConnectString(SessionName, JoinAddress);
					if (Local_bIsDedicatedServerSession)
					{
						TArray<FString> IpPortArray;
						JoinAddress.ParseIntoArray(IpPortArray, TEXT(":"), true);
						const FString IpAddress = IpPortArray[0];
						if (LocalPortInfo.IsEmpty())
						{
							LocalPortInfo = TEXT("7777");
						}
						const FString NewCustomIP = IpAddress + TEXT(":") + LocalPortInfo;
						JoinAddress = NewCustomIP;
					}
					if (!JoinAddress.IsEmpty())
					{
						PlayerControllerRef->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
						JoinSession_CallbackBP.ExecuteIfBound(true);
						return;
					}
					else
					{
						JoinSession_CallbackBP.ExecuteIfBound(false);
						return;
					}
				}
				else
				{
					JoinSession_CallbackBP.ExecuteIfBound(false);
					return;
				}
			}
			else
			{
				JoinSession_CallbackBP.ExecuteIfBound(false);
				return;
			}
		}
		else
		{
			JoinSession_CallbackBP.ExecuteIfBound(false);
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Join Session Error with Reason of %d"), Result);
		JoinSession_CallbackBP.ExecuteIfBound(false);
		return;
	}
	JoinSession_CallbackBP.ExecuteIfBound(false);
}

void UEOSKitSubsystem::OnDestroySessionCompleted(FName SessionName, bool bWasSuccess) const
{
	DestroySession_CallbackBP.ExecuteIfBound(bWasSuccess);
}

void UEOSKitSubsystem::OnUpdateStatsCompleted(const FOnlineError& Result) const
{
	if (Result == FOnlineError::Success())
	{
		UpdateStat_CallbackBP.ExecuteIfBound(true);
	}
	else
	{
		UpdateStat_CallbackBP.ExecuteIfBound(false);
	}
}

void UEOSKitSubsystem::OnGetStatsCompleted(const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult) const
{
	if (ResultState.WasSuccessful())
	{
		TArray<FEOSKitStats> LocalStatsArray;
		for (const auto& StatsVar : UsersStatsResult)
		{
			for (auto StoredValueRef : StatsVar->Stats)
			{
				FString Keyname = StoredValueRef.Key;
				int32 Value;
				StoredValueRef.Value.GetValue(Value);
				FEOSKitStats LocalStats;
				LocalStats.StatsName = Keyname;
				LocalStats.StatsValue = FString::FromInt(Value);
				LocalStatsArray.Add(LocalStats);
			}
		}
		GetStats_CallbackBP.ExecuteIfBound(true, LocalStatsArray);
	}
	else
	{
		GetStats_CallbackBP.ExecuteIfBound(false, TArray<FEOSKitStats>());
		UE_LOG(LogTemp, Warning, TEXT("Getting stats failed with error - %s"), *ResultState.ToLogString());
	}
}

void UEOSKitSubsystem::OnWriteFileComplete(bool bSuccess, const FUniqueNetId& UserID, const FString& FileName) const
{
	WriteFile_CallbackBP.ExecuteIfBound(true);
}

void UEOSKitSubsystem::OnGetFileComplete(bool bSuccess, const FUniqueNetId& UserID, const FString& FileName) const
{
	if (bSuccess)
	{
		if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
		{
			if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				if (const IOnlineUserCloudPtr CloudPointerRef = SubsystemRef->GetUserCloudInterface())
				{
					TSharedPtr<const FUniqueNetId> UserIDRef = IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef();
					TArray<uint8> FileContents;
					CloudPointerRef->GetFileContents(*UserIDRef, FileName, FileContents);
					if (FileContents.Num() > 0)
					{
						USaveGame* LocalSaveGame = UGameplayStatics::LoadGameFromMemory(FileContents);
						GetFile_CallbackBP.ExecuteIfBound(true, LocalSaveGame);
					}
					else
					{
						GetFile_CallbackBP.ExecuteIfBound(false, nullptr);
					}
				}
				else
				{
					GetFile_CallbackBP.ExecuteIfBound(false, nullptr);
				}
			}
			else
			{
				GetFile_CallbackBP.ExecuteIfBound(false, nullptr);
			}
		}
		else
		{
			GetFile_CallbackBP.ExecuteIfBound(false, nullptr);
		}
	}
	else
	{
		GetFile_CallbackBP.ExecuteIfBound(false, nullptr);
	}
}

void UEOSKitSubsystem::OnTitleFileListComplete(bool bSuccess, const FString& Error) const
{
	TitleFileList_CallbackBP.ExecuteIfBound(bSuccess, Error);
}

void UEOSKitSubsystem::OnTitleFileComplete(bool bSuccess, const FString& FileName) const
{
	GetTitleFile_CallbackBP.ExecuteIfBound(bSuccess);
}

void UEOSKitSubsystem::OnLeaderboardListCompleted(bool bWasSuccess) const
{
	// Leaderboard read completed - can be extended if needed
}

// ========================================
// EOS SDK Access
// ========================================

EOS_HPlatform UEOSKitSubsystem::GetPlatformHandle() const
{
#if WITH_EOS_SDK
	if (IEOSSDKManager* SDKManager = IEOSSDKManager::Get())
	{
		return static_cast<EOS_HPlatform>(SDKManager->GetPlatformHandle());
	}
#endif
	return nullptr;
}

EOS_ProductUserId UEOSKitSubsystem::GetProductUserId(int32 LocalUserNum) const
{
#if WITH_EOS_SDK
	// DO NOT call back into IOnlineIdentity - it will cause infinite recursion!
	// Instead, query the Connect interface directly from the EOS platform handle
	EOS_HPlatform PlatformHandle = GetPlatformHandle();
	if (PlatformHandle)
	{
		EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
		if (ConnectHandle)
		{
			// Get the logged in ProductUserId for this local user
			// Note: This assumes the user has logged in via Connect interface
			// For a more robust implementation, you'd cache the ProductUserId during login
			return EOS_Connect_GetLoggedInUserByIndex(ConnectHandle, LocalUserNum);
		}
	}
#endif
	return nullptr;
}

EOS_HUserInfo UEOSKitSubsystem::GetUserInfoHandle() const
{
#if WITH_EOS_SDK
	EOS_HPlatform PlatformHandle = GetPlatformHandle();
	if (PlatformHandle)
	{
		return EOS_Platform_GetUserInfoInterface(PlatformHandle);
	}
#endif
	return nullptr;
}