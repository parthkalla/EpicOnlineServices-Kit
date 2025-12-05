// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitSimpleSubsystem.h"
#include "EOSKitOnlineHelpers.h"
#include "EOSKitSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"

void UEOSKitSimpleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Simple Subsystem Initialized"));
}

void UEOSKitSimpleSubsystem::Deinitialize()
{
	// Clean up any active sessions before shutting down
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
		{
			// Try to destroy common session names
			TArray<FName> CommonSessionNames = { NAME_GameSession, NAME_PartySession, FName(TEXT("Modified_EOS_Session")), FName(TEXT("Modified_EOS_Lobby")) };
			for (const FName& SessionName : CommonSessionNames)
			{
				if (FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName))
				{
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Destroying session '%s' on subsystem shutdown"), *SessionName.ToString());
					SessionInterface->DestroySession(SessionName);
				}
			}
		}
	}

	bIsInitialized = false;
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Simple Subsystem Deinitialized"));
	Super::Deinitialize();
}

// ========================================
// Authentication
// ========================================

void UEOSKitSimpleSubsystem::LoginWithDeviceID(const FString& DisplayName, const FString& DeviceName, const FEOSKit_Login_Callback& Result)
{
	UE_LOG(LogTemp, Verbose, TEXT("EOSKit: LoginWithDeviceID called - DisplayName: %s, DeviceName: %s"), *DisplayName, *DeviceName);

	if (UEOSKitSubsystem* EOSKit = GetEOSKitSubsystem())
	{
		StoredLoginCallback = Result;

		FBP_EOSKit_Login_Callback InternalCallback;
		InternalCallback.BindUFunction(this, FName("HandleLoginResult"));
		EOSKit->LoginWithDeviceID(0, DisplayName, DeviceName, InternalCallback);
		return;
	}

	Result.ExecuteIfBound(false, TEXT("EOSKit subsystem not available"));
}

void UEOSKitSimpleSubsystem::LoginWithAccountPortal(const FEOSKit_Login_Callback& Result)
{
	UE_LOG(LogTemp, Verbose, TEXT("EOSKit: LoginWithAccountPortal called"));

	if (UEOSKitSubsystem* EOSKit = GetEOSKitSubsystem())
	{
		StoredLoginCallback = Result;

		FBP_EOSKit_Login_Callback InternalCallback;
		InternalCallback.BindUFunction(this, FName("HandleLoginResult"));
		EOSKit->LoginWithAccountPortal(0, InternalCallback);
		return;
	}

	Result.ExecuteIfBound(false, TEXT("EOSKit subsystem not available"));
}

void UEOSKitSimpleSubsystem::LoginWithPersistentAuth(const FEOSKit_Login_Callback& Result)
{
	UE_LOG(LogTemp, Verbose, TEXT("EOSKit: LoginWithPersistentAuth called"));

	if (UEOSKitSubsystem* EOSKit = GetEOSKitSubsystem())
	{
		StoredLoginCallback = Result;

		FBP_EOSKit_Login_Callback InternalCallback;
		InternalCallback.BindUFunction(this, FName("HandleLoginResult"));
		EOSKit->LoginWithPersistantAuth(0, InternalCallback);
		return;
	}

	Result.ExecuteIfBound(false, TEXT("EOSKit subsystem not available"));
}

void UEOSKitSimpleSubsystem::Logout(const FEOSKit_Logout_Callback& Result)
{
	UE_LOG(LogTemp, Verbose, TEXT("EOSKit: Logout called"));

	if (UEOSKitSubsystem* EOSKit = GetEOSKitSubsystem())
	{
		StoredLogoutCallback = Result;

		FBP_EOSKit_Logout_Callback InternalCallback;
		InternalCallback.BindUFunction(this, FName("HandleLogoutResult"));
		EOSKit->Logout(0, InternalCallback);
		return;
	}

	Result.ExecuteIfBound(false);
}

// ========================================
// User Information
// ========================================

FString UEOSKitSimpleSubsystem::GetPlayerNickname(int32 LocalUserNum) const
{
	return FEOSKitOnlineHelpers::GetPlayerNickname(LocalUserNum);
}

bool UEOSKitSimpleSubsystem::GetLoginStatus(int32 LocalUserNum) const
{
	return FEOSKitOnlineHelpers::IsPlayerLoggedIn(LocalUserNum);
}

FString UEOSKitSimpleSubsystem::GetProductUserID(int32 LocalUserNum) const
{
	return FEOSKitOnlineHelpers::GetProductUserId(LocalUserNum);
}

FString UEOSKitSimpleSubsystem::GetEpicID(int32 LocalUserNum) const
{
	return FEOSKitOnlineHelpers::GetEpicAccountId(LocalUserNum);
}

// ========================================
// Sessions
// ========================================

void UEOSKitSimpleSubsystem::CreateEOSSession(
	const TMap<FString, FString>& CustomSettings,
	const FString& SessionName,
	bool bIsDedicatedServer,
	bool bIsLan,
	int32 NumberOfPublicConnections,
	EEOSKitRegion Region,
	const FEOSKit_CreateSession_Callback& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: CreateEOSSession called - SessionName: %s, Connections: %d"), *SessionName, NumberOfPublicConnections);
	
	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateEOSSession - WorldContext is null"));
		Result.ExecuteIfBound(false, NAME_None);
		return;
	}

	if (UEOSKitSubsystem* EOSKit = GetEOSKitSubsystem())
	{
		StoredCreateSessionCallback = Result;

		FBP_EOSKit_CreateSession_Callback InternalCallback;
		InternalCallback.BindUFunction(this, FName("HandleCreateSessionResult"));

		EOSKit->CreateEOSSession(
			InternalCallback,
			CustomSettings,
			SessionName,
			bIsDedicatedServer,
			bIsLan,
			NumberOfPublicConnections,
			Region
		);
	}
	else
	{
		Result.ExecuteIfBound(false, NAME_None);
	}
}

void UEOSKitSimpleSubsystem::CreateEOSLobby(
	const TMap<FString, FString>& CustomSettings,
	const FString& SessionName,
	bool bUseVoiceChat,
	bool bUsePresence,
	bool bAllowInvites,
	bool bAdvertise,
	bool bAllowJoinInProgress,
	bool bIsLan,
	int32 NumberOfPublicConnections,
	int32 NumberOfPrivateConnections,
	const FEOSKit_CreateLobby_Callback& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: CreateEOSLobby called - SessionName: %s, Connections: %d"), *SessionName, NumberOfPublicConnections);
	
	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateEOSLobby - WorldContext is null"));
		Result.ExecuteIfBound(false, NAME_None);
		return;
	}

	if (UEOSKitSubsystem* EOSKit = GetEOSKitSubsystem())
	{
		StoredCreateLobbyCallback = Result;

		FBP_EOSKit_CreateLobby_Callback InternalCallback;
		InternalCallback.BindUFunction(this, FName("HandleCreateLobbyResult"));

		EOSKit->CreateEOSLobby(
			InternalCallback,
			CustomSettings,
			SessionName,
			bUseVoiceChat,
			bUsePresence,
			bAllowInvites,
			bAdvertise,
			bAllowJoinInProgress,
			bIsLan,
			NumberOfPublicConnections,
			NumberOfPrivateConnections
		);
	}
	else
	{
		Result.ExecuteIfBound(false, NAME_None);
	}
}

void UEOSKitSimpleSubsystem::FindEOSSession(
	const TMap<FString, FString>& SearchSettings,
	EEOSKitMatchType MatchType,
	EEOSKitRegion RegionToSearch,
	const FEOSKit_FindSession_Callback& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: FindEOSSession called - MatchType: %d"), static_cast<int32>(MatchType));
	
	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: FindEOSSession - WorldContext is null"));
		Result.ExecuteIfBound(false, TArray<FString>());
		return;
	}

	// Convert SearchSettings to FEOSKitAttribute map
	TMap<FString, FEOSKitAttribute> SearchAttributes;
	for (const auto& Setting : SearchSettings)
	{
		FEOSKitAttribute Attr;
		Attr.StringValue = Setting.Value;
		SearchAttributes.Add(Setting.Key, Attr);
	}

	// Use FindLobbies for now (can be extended to find sessions)
	if (MatchType == EEOSKitMatchType::LobbySession)
	{
		// TODO: Implement FindLobbiesAsync wrapper
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: FindEOSSession - FindLobbies not yet implemented in SimpleSubsystem"));
		Result.ExecuteIfBound(false, TArray<FString>());
	}
	else
	{
		// TODO: Implement FindSessionsAsync wrapper
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: FindEOSSession - FindSessions not yet implemented in SimpleSubsystem"));
		Result.ExecuteIfBound(false, TArray<FString>());
	}
}

void UEOSKitSimpleSubsystem::DestroyEOSSession(const FName& SessionName, const FEOSKit_DestroySession_Callback& Result)
{
	UE_LOG(LogTemp, Verbose, TEXT("EOSKit: DestroyEOSSession called - SessionName: %s"), *SessionName.ToString());

	if (UEOSKitSubsystem* EOSKit = GetEOSKitSubsystem())
	{
		StoredDestroySessionCallback = Result;

		FBP_EOSKit_DestroySession_Callback InternalCallback;
		InternalCallback.BindUFunction(this, FName("HandleDestroySessionResult"));

		EOSKit->DestroyEosSession(InternalCallback, SessionName);
	}
	else
	{
		Result.ExecuteIfBound(false);
	}
}

void UEOSKitSimpleSubsystem::JoinEOSSession(
	const FName& SessionName,
	const FString& SessionId,
	bool bIsDedicatedServerSession,
	const FEOSKit_JoinSession_Callback& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: JoinEOSSession called - SessionName: %s, SessionId: %s"), *SessionName.ToString(), *SessionId);
	
	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: JoinEOSSession - WorldContext is null"));
		Result.ExecuteIfBound(false);
		return;
	}

	// TODO: Implement JoinLobbyAsync wrapper
	// For now, use OnlineSubsystem
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			// Create a search result from SessionId
			// This is a simplified version - full implementation would need proper session search result
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: JoinEOSSession - Using OnlineSubsystem (simplified)"));
			Result.ExecuteIfBound(false); // Not fully implemented yet
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("EOSKit: JoinEOSSession - Failed to get session interface"));
	Result.ExecuteIfBound(false);
}

// ========================================
// Session Management
// ========================================

void UEOSKitSimpleSubsystem::RegisterPlayer(const FName& SessionName, bool bWasInvited)
{
	FEOSKitOnlineHelpers::RegisterLocalPlayer(GetWorld(), SessionName, bWasInvited);
}

void UEOSKitSimpleSubsystem::UnregisterPlayer(const FName& SessionName)
{
	FEOSKitOnlineHelpers::UnregisterLocalPlayer(GetWorld(), SessionName);
}

bool UEOSKitSimpleSubsystem::StartSession(const FName& SessionName)
{
	return FEOSKitOnlineHelpers::StartSession(GetWorld(), SessionName);
}

bool UEOSKitSimpleSubsystem::EndSession(const FName& SessionName)
{
	return FEOSKitOnlineHelpers::EndSession(GetWorld(), SessionName);
}

// ========================================
// Social Features
// ========================================

bool UEOSKitSimpleSubsystem::ShowFriendUserInterface()
{
	if (UEOSKitSubsystem* EOSKit = GetEOSKitSubsystem())
	{
		return EOSKit->ShowFriendUserInterface();
	}

	return FEOSKitOnlineHelpers::ShowFriendsInterface();
}

// ========================================
// Utility Functions
// ========================================

FString UEOSKitSimpleSubsystem::GenerateSessionCode(int32 CodeLength) const
{
	return FEOSKitOnlineHelpers::GenerateSessionCode(CodeLength);
}

bool UEOSKitSimpleSubsystem::IsEOSKitInitialized() const
{
	return FEOSKitOnlineHelpers::IsEOSKitInitialized();
}

// ========================================
// Helper Functions for Delegate Callbacks
// ========================================

UEOSKitSubsystem* UEOSKitSimpleSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}

	return nullptr;
}

void UEOSKitSimpleSubsystem::HandleLoginResult(bool bWasSuccess, const FString& Error)
{
	StoredLoginCallback.ExecuteIfBound(bWasSuccess, Error);
}

void UEOSKitSimpleSubsystem::HandleLogoutResult(bool bWasSuccess)
{
	StoredLogoutCallback.ExecuteIfBound(bWasSuccess);
}

void UEOSKitSimpleSubsystem::HandleCreateSessionResult(bool bWasSuccess, const FName& SessionName)
{
	StoredCreateSessionCallback.ExecuteIfBound(bWasSuccess, SessionName);
}

void UEOSKitSimpleSubsystem::HandleCreateLobbyResult(bool bWasSuccess, const FName& SessionName)
{
	StoredCreateLobbyCallback.ExecuteIfBound(bWasSuccess, SessionName);
}

void UEOSKitSimpleSubsystem::HandleDestroySessionResult(bool bWasSuccess)
{
	StoredDestroySessionCallback.ExecuteIfBound(bWasSuccess);
}

