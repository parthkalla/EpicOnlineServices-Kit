// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitSimpleSubsystem.h"
#include "EOSKitSubsystem.h"
#include "EOSKitGameInstanceSubsystem.h"
#include "EOSCreateEOKLobbyAsync.h"
#include "EOSCreateEOKSessionAsync.h"
#include "EOSFindLobbiesAsync.h"
#include "EOSJoinLobbyAsync.h"
#include "EOSLoginAsync.h"
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
	// Clean up any active async nodes
	ActiveAsyncNodes.Empty();
	bIsInitialized = false;
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Simple Subsystem Deinitialized"));
	Super::Deinitialize();
}

// ========================================
// Authentication
// ========================================

void UEOSKitSimpleSubsystem::LoginWithDeviceID(const FString& DisplayName, const FString& DeviceName, const FEOSKit_Login_Callback& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: LoginWithDeviceID called - DisplayName: %s, DeviceName: %s"), *DisplayName, *DeviceName);
	
	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithDeviceID - WorldContext is null"));
		Result.ExecuteIfBound(false, TEXT("WorldContext is null"));
		return;
	}

	// Use Connect Interface for Device ID login
	UEOSLoginUsingConnectInterface* LoginNode = UEOSLoginUsingConnectInterface::LoginUsingConnectInterface(
		WorldContext,
		TEXT("deviceid"),
		DisplayName,
		DeviceName
	);

	if (!LoginNode)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithDeviceID - Failed to create login node"));
		Result.ExecuteIfBound(false, TEXT("Failed to create login node"));
		return;
	}

	ActiveAsyncNodes.Add(LoginNode);

	LoginNode->OnSuccess.AddLambda([this, Result, LoginNode](const FString& EpicUserId, const FString& ProductUserId, const FString& Error)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: LoginWithDeviceID SUCCESS - ProductUserId: %s"), *ProductUserId);
		ActiveAsyncNodes.Remove(LoginNode);
		Result.ExecuteIfBound(true, TEXT(""));
	});

	LoginNode->OnFail.AddLambda([this, Result, LoginNode](const FString& EpicUserId, const FString& ProductUserId, const FString& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithDeviceID FAILED - Error: %s"), *Error);
		ActiveAsyncNodes.Remove(LoginNode);
		Result.ExecuteIfBound(false, Error);
	});

	LoginNode->Activate();
}

void UEOSKitSimpleSubsystem::LoginWithAccountPortal(const FEOSKit_Login_Callback& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: LoginWithAccountPortal called"));
	
	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithAccountPortal - WorldContext is null"));
		Result.ExecuteIfBound(false, TEXT("WorldContext is null"));
		return;
	}

	// Use Auth Interface for Account Portal login
	UEOSLoginUsingAuthInterface* LoginNode = UEOSLoginUsingAuthInterface::LoginUsingAuthInterface(
		WorldContext,
		EEOSCredentialType::AccountPortal,
		EEOSExternalCredentialType::Epic,
		TEXT(""),
		TEXT("")
	);

	if (!LoginNode)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithAccountPortal - Failed to create login node"));
		Result.ExecuteIfBound(false, TEXT("Failed to create login node"));
		return;
	}

	ActiveAsyncNodes.Add(LoginNode);

	LoginNode->OnSuccess.AddLambda([this, Result, LoginNode](const FString& EpicUserId, const FString& ProductUserId, const FString& Error)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: LoginWithAccountPortal SUCCESS - ProductUserId: %s"), *ProductUserId);
		ActiveAsyncNodes.Remove(LoginNode);
		Result.ExecuteIfBound(true, TEXT(""));
	});

	LoginNode->OnFail.AddLambda([this, Result, LoginNode](const FString& EpicUserId, const FString& ProductUserId, const FString& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithAccountPortal FAILED - Error: %s"), *Error);
		ActiveAsyncNodes.Remove(LoginNode);
		Result.ExecuteIfBound(false, Error);
	});

	LoginNode->Activate();
}

void UEOSKitSimpleSubsystem::LoginWithPersistentAuth(const FEOSKit_Login_Callback& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: LoginWithPersistentAuth called"));
	
	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithPersistentAuth - WorldContext is null"));
		Result.ExecuteIfBound(false, TEXT("WorldContext is null"));
		return;
	}

	// Use Auth Interface for Persistent Auth login
	UEOSLoginUsingAuthInterface* LoginNode = UEOSLoginUsingAuthInterface::LoginUsingAuthInterface(
		WorldContext,
		EEOSCredentialType::PersistentAuth,
		EEOSExternalCredentialType::Epic,
		TEXT(""),
		TEXT("")
	);

	if (!LoginNode)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithPersistentAuth - Failed to create login node"));
		Result.ExecuteIfBound(false, TEXT("Failed to create login node"));
		return;
	}

	ActiveAsyncNodes.Add(LoginNode);

	LoginNode->OnSuccess.AddLambda([this, Result, LoginNode](const FString& EpicUserId, const FString& ProductUserId, const FString& Error)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: LoginWithPersistentAuth SUCCESS - ProductUserId: %s"), *ProductUserId);
		ActiveAsyncNodes.Remove(LoginNode);
		Result.ExecuteIfBound(true, TEXT(""));
	});

	LoginNode->OnFail.AddLambda([this, Result, LoginNode](const FString& EpicUserId, const FString& ProductUserId, const FString& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: LoginWithPersistentAuth FAILED - Error: %s"), *Error);
		ActiveAsyncNodes.Remove(LoginNode);
		Result.ExecuteIfBound(false, Error);
	});

	LoginNode->Activate();
}

void UEOSKitSimpleSubsystem::Logout(const FEOSKit_Logout_Callback& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Logout called"));
	
	// For now, just clear the ProductUserId and return success
	// Full logout implementation would require EOS Auth logout
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
		if (EOSKitSubsystem)
		{
			EOSKitSubsystem->SetProductUserId(nullptr);
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: Logout - ProductUserId cleared"));
		}
	}

	Result.ExecuteIfBound(true);
}

// ========================================
// User Information
// ========================================

FString UEOSKitSimpleSubsystem::GetPlayerNickname(int32 LocalUserNum) const
{
	return UEOSKitGameInstanceSubsystem::GetPlayerNickname(LocalUserNum);
}

bool UEOSKitSimpleSubsystem::GetLoginStatus(int32 LocalUserNum) const
{
	return UEOSKitGameInstanceSubsystem::IsPlayerLoggedIn(LocalUserNum);
}

FString UEOSKitSimpleSubsystem::GetProductUserID(int32 LocalUserNum) const
{
	return UEOSKitGameInstanceSubsystem::GetProductUserId(LocalUserNum);
}

FString UEOSKitSimpleSubsystem::GetEpicID(int32 LocalUserNum) const
{
	return UEOSKitGameInstanceSubsystem::GetEpicAccountId(LocalUserNum);
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

	// Convert CustomSettings to FEOSKitAttribute map
	TMap<FString, FEOSKitAttribute> SessionSettings;
	for (const auto& Setting : CustomSettings)
	{
		FEOSKitAttribute Attr;
		Attr.StringValue = Setting.Value;
		SessionSettings.Add(Setting.Key, Attr);
	}

	// Create ExtraSettings struct
	FEOSKitCreateSessionSettings ExtraSettings;
	ExtraSettings.SessionName = SessionName;
	ExtraSettings.NumberOfPublicConnections = NumberOfPublicConnections;
	ExtraSettings.bIsLanMatch = bIsLan;
	ExtraSettings.bIsDedicatedServer = bIsDedicatedServer;
	ExtraSettings.bUsePresence = true; // Required for P2P
	ExtraSettings.bShouldAdvertise = true;
	ExtraSettings.Region = Region;
	ExtraSettings.BucketId = TEXT("EOSKitBucket"); // Default bucket

	FEOSKitDedicatedServerSettings DedicatedServerSettings;

	UEOSCreateEOKSessionAsync* CreateSessionNode = UEOSCreateEOKSessionAsync::CreateEOKSession(
		WorldContext,
		SessionSettings,
		FName(*SessionName),
		NumberOfPublicConnections,
		DedicatedServerSettings,
		ExtraSettings
	);

	if (!CreateSessionNode)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateEOSSession - Failed to create session node"));
		Result.ExecuteIfBound(false, NAME_None);
		return;
	}

	ActiveAsyncNodes.Add(CreateSessionNode);

	CreateSessionNode->OnSuccess.AddLambda([this, Result, SessionName, CreateSessionNode](const FString& SessionId)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: CreateEOSSession SUCCESS - SessionId: %s"), *SessionId);
		ActiveAsyncNodes.Remove(CreateSessionNode);
		Result.ExecuteIfBound(true, FName(*SessionName));
	});

	CreateSessionNode->OnFail.AddLambda([this, Result, SessionName, CreateSessionNode](const FString& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateEOSSession FAILED - Error: %s"), *Error);
		ActiveAsyncNodes.Remove(CreateSessionNode);
		Result.ExecuteIfBound(false, FName(*SessionName));
	});

	CreateSessionNode->Activate();
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

	// Convert CustomSettings to FEOSKitAttribute maps
	TMap<FString, FEOSKitAttribute> SessionSettings;
	TMap<FString, FEOSKitAttribute> MemberSettings;
	
	for (const auto& Setting : CustomSettings)
	{
		FEOSKitAttribute Attr;
		Attr.StringValue = Setting.Value;
		SessionSettings.Add(Setting.Key, Attr);
	}

	// Create ExtraSettings struct
	FEOSKitCreateLobbySettings ExtraSettings;
	ExtraSettings.SessionName = SessionName;
	ExtraSettings.NumberOfPublicConnections = NumberOfPublicConnections;
	ExtraSettings.NumberOfPrivateConnections = NumberOfPrivateConnections;
	ExtraSettings.bIsLanMatch = bIsLan;
	ExtraSettings.bAllowInvites = bAllowInvites;
	ExtraSettings.bShouldAdvertise = bAdvertise;
	ExtraSettings.bAllowJoinInProgress = bAllowJoinInProgress;
	ExtraSettings.bUseVoiceChat = bUseVoiceChat;
	ExtraSettings.bUsePresence = bUsePresence;
	ExtraSettings.BucketId = TEXT("EOSKitBucket"); // Default bucket

	UEOSCreateEOKLobbyAsync* CreateLobbyNode = UEOSCreateEOKLobbyAsync::CreateEOKLobby(
		WorldContext,
		SessionSettings,
		MemberSettings,
		FName(*SessionName),
		NumberOfPublicConnections,
		ExtraSettings
	);

	if (!CreateLobbyNode)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateEOSLobby - Failed to create lobby node"));
		Result.ExecuteIfBound(false, NAME_None);
		return;
	}

	ActiveAsyncNodes.Add(CreateLobbyNode);

	CreateLobbyNode->OnSuccess.AddLambda([this, Result, SessionName, CreateLobbyNode](const FString& LobbyId)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: CreateEOSLobby SUCCESS - LobbyId: %s"), *LobbyId);
		ActiveAsyncNodes.Remove(CreateLobbyNode);
		Result.ExecuteIfBound(true, FName(*SessionName));
	});

	CreateLobbyNode->OnFail.AddLambda([this, Result, SessionName, CreateLobbyNode](const FString& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateEOSLobby FAILED - Error: %s"), *Error);
		ActiveAsyncNodes.Remove(CreateLobbyNode);
		Result.ExecuteIfBound(false, FName(*SessionName));
	});

	CreateLobbyNode->Activate();
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
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: DestroyEOSSession called - SessionName: %s"), *SessionName.ToString());
	
	// Use OnlineSubsystem for session destruction
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			SessionPtr->OnDestroySessionCompleteDelegates.AddLambda([Result](FName SessionName, bool bWasSuccessful)
			{
				Result.ExecuteIfBound(bWasSuccessful);
			});
			SessionPtr->DestroySession(SessionName);
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("EOSKit: DestroyEOSSession - Failed to get session interface"));
	Result.ExecuteIfBound(false);
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
	UEOSKitGameInstanceSubsystem* GameInstanceSubsystem = GetGameInstance()->GetSubsystem<UEOSKitGameInstanceSubsystem>();
	if (GameInstanceSubsystem)
	{
		GameInstanceSubsystem->RegisterLocalPlayer(SessionName, bWasInvited);
	}
}

void UEOSKitSimpleSubsystem::UnregisterPlayer(const FName& SessionName)
{
	UEOSKitGameInstanceSubsystem* GameInstanceSubsystem = GetGameInstance()->GetSubsystem<UEOSKitGameInstanceSubsystem>();
	if (GameInstanceSubsystem)
	{
		GameInstanceSubsystem->UnregisterLocalPlayer(SessionName);
	}
}

bool UEOSKitSimpleSubsystem::StartSession(const FName& SessionName)
{
	UEOSKitGameInstanceSubsystem* GameInstanceSubsystem = GetGameInstance()->GetSubsystem<UEOSKitGameInstanceSubsystem>();
	if (GameInstanceSubsystem)
	{
		return GameInstanceSubsystem->StartSession(SessionName);
	}
	return false;
}

bool UEOSKitSimpleSubsystem::EndSession(const FName& SessionName)
{
	UEOSKitGameInstanceSubsystem* GameInstanceSubsystem = GetGameInstance()->GetSubsystem<UEOSKitGameInstanceSubsystem>();
	if (GameInstanceSubsystem)
	{
		return GameInstanceSubsystem->EndSession(SessionName);
	}
	return false;
}

// ========================================
// Social Features
// ========================================

bool UEOSKitSimpleSubsystem::ShowFriendUserInterface()
{
	return UEOSKitGameInstanceSubsystem::ShowFriendsInterface();
}

// ========================================
// Utility Functions
// ========================================

FString UEOSKitSimpleSubsystem::GenerateSessionCode(int32 CodeLength) const
{
	return UEOSKitGameInstanceSubsystem::GenerateSessionCode(CodeLength);
}

bool UEOSKitSimpleSubsystem::IsEOSKitInitialized() const
{
	return UEOSKitGameInstanceSubsystem::IsEOSKitInitialized();
}

