// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitGameInstanceSubsystem.h"
#include "EOSKitSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

#if WITH_EOS_SDK
	#include "eos_sdk.h"
	#include "eos_sessions.h"
	#include "eos_sessions_types.h"
	#include "eos_common.h"
#endif

void UEOSKitGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	bIsInitialized = true;
	
	// Listen for level changes to auto-register players in active sessions
	WorldInitDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UEOSKitGameInstanceSubsystem::OnWorldInitialized);
	
	// Register for standalone game shutdown (works in packaged builds)
	OnExitDelegateHandle = FCoreDelegates::OnExit.AddUObject(this, &UEOSKitGameInstanceSubsystem::OnStandaloneGameShutdown);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Game Instance Subsystem Initialized (Standalone mode cleanup registered)"));
}

void UEOSKitGameInstanceSubsystem::OnWorldInitialized(UWorld* World, UWorld::InitializationValues IVS)
{
	// DISABLED: Auto-registration causes 403 errors with P2P policy
	// For P2P sessions, EOS automatically registers the host when StartSession is called
	// Manual RegisterPlayer calls fail with P2P policy (missing matchmaking:managePlayers permission)
	//
	// If you're using Matchmaking policy (not P2P), you can re-enable this:
	/*
	if (World && World->GetNetMode() != NM_DedicatedServer)
	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, [World]()
		{
			if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World))
			{
				if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
				{
					if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
					{
						TArray<FName> SessionNames = { 
							NAME_GameSession, 
							NAME_PartySession
						};
						
						for (const FName& SessionName : SessionNames)
						{
							if (FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName))
							{
								if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(0))
								{
									SessionInterface->RegisterPlayer(SessionName, *UniqueId, false);
								}
							}
						}
					}
				}
			}
		}, 0.1f, false);
	}
	*/
}

void UEOSKitGameInstanceSubsystem::Deinitialize()
{
	// Remove world initialization delegate
	if (WorldInitDelegateHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(WorldInitDelegateHandle);
		WorldInitDelegateHandle.Reset();
	}
	
	// Remove standalone exit delegate
	if (OnExitDelegateHandle.IsValid())
	{
		FCoreDelegates::OnExit.Remove(OnExitDelegateHandle);
		OnExitDelegateHandle.Reset();
	}
	
	bIsInitialized = false;
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Game Instance Subsystem Deinitialized"));
	
	Super::Deinitialize();
}

// ========================================
// User Information
// ========================================

bool UEOSKitGameInstanceSubsystem::IsPlayerLoggedIn(int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
		{
			return Identity->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn;
		}
	}
	return false;
}

FString UEOSKitGameInstanceSubsystem::GetProductUserId(int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
		{
			if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(LocalUserNum))
			{
				return ParseProductUserId(UniqueId->ToString());
			}
		}
	}
	return FString();
}

FString UEOSKitGameInstanceSubsystem::GetEpicAccountId(int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
		{
			if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(LocalUserNum))
			{
				return ParseEpicAccountId(UniqueId->ToString());
			}
		}
	}
	return FString();
}

// ========================================
// Session Utilities
// ========================================

FString UEOSKitGameInstanceSubsystem::GenerateSessionCode(int32 CodeLength)
{
	const FString Characters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	FString Code;
	Code.Reserve(CodeLength);
	
	for (int32 i = 0; i < CodeLength; i++)
	{
		const int32 RandomIndex = FMath::RandRange(0, Characters.Len() - 1);
		Code.AppendChar(Characters[RandomIndex]);
	}
	
	return Code;
}

void UEOSKitGameInstanceSubsystem::RegisterLocalPlayer(FName SessionName, bool bWasInvited)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr Session = Subsystem->GetSessionInterface())
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(0))
				{
					if (Session->RegisterPlayer(SessionName, *UniqueId, bWasInvited))
					{
						UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully registered player in session '%s'"), *SessionName.ToString());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to register player in session '%s'"), *SessionName.ToString());
					}
				}
			}
		}
	}
}

void UEOSKitGameInstanceSubsystem::UnregisterLocalPlayer(FName SessionName)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr Session = Subsystem->GetSessionInterface())
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				if (TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(0))
				{
					if (Session->UnregisterPlayer(SessionName, *UniqueId))
					{
						UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully unregistered player from session '%s'"), *SessionName.ToString());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to unregister player from session '%s'"), *SessionName.ToString());
					}
				}
			}
		}
	}
}

bool UEOSKitGameInstanceSubsystem::StartSession(FName SessionName)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr Session = Subsystem->GetSessionInterface())
		{
			if (Session->StartSession(SessionName))
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully started session '%s'"), *SessionName.ToString());
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to start session '%s'"), *SessionName.ToString());
			}
		}
	}
	return false;
}

bool UEOSKitGameInstanceSubsystem::EndSession(FName SessionName)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr Session = Subsystem->GetSessionInterface())
		{
			if (Session->EndSession(SessionName))
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully ended session '%s'"), *SessionName.ToString());
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to end session '%s'"), *SessionName.ToString());
			}
		}
	}
	return false;
}

// ========================================
// Social Features
// ========================================

bool UEOSKitGameInstanceSubsystem::ShowFriendsInterface()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		if (const IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface())
		{
			if (ExternalUI->ShowFriendsUI(0))
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully opened friends interface"));
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to open friends interface"));
			}
		}
	}
	return false;
}

// ========================================
// Helper Functions
// ========================================

FString UEOSKitGameInstanceSubsystem::ParseProductUserId(const FString& UniqueNetIdString)
{
	// EOS format: "EpicAccountId|ProductUserId"
	TArray<FString> Parts;
	UniqueNetIdString.ParseIntoArray(Parts, TEXT("|"));
	
	if (Parts.Num() == 2)
	{
		// Return the ProductUserId (second part)
		return Parts[1];
	}
	else if (Parts.Num() == 1)
	{
		// If there's no pipe, assume it's just the ProductUserId
		return Parts[0];
	}
	
	return FString();
}

FString UEOSKitGameInstanceSubsystem::ParseEpicAccountId(const FString& UniqueNetIdString)
{
	// EOS format: "EpicAccountId|ProductUserId"
	TArray<FString> Parts;
	UniqueNetIdString.ParseIntoArray(Parts, TEXT("|"));
	
	if (Parts.Num() >= 2)
	{
		// Return the EpicAccountId (first part)
		return Parts[0];
	}
	
	return FString();
}

bool UEOSKitGameInstanceSubsystem::IsEOSKitInitialized()
{
	// Check if the main EOSKit subsystem is initialized
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(nullptr))
	{
		// Check if it's the EOS subsystem
		if (Subsystem->GetSubsystemName() == TEXT("EOS") || 
			Subsystem->GetSubsystemName() == TEXT("EOSPlus"))
		{
			// Verify we can get the identity interface
			if (Subsystem->GetIdentityInterface().IsValid())
			{
				return true;
			}
		}
	}
	
	return false;
}

int32 UEOSKitGameInstanceSubsystem::DestroyAllActiveSessions()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Manually destroying all active sessions..."));
	
	int32 DestroyedCount = 0;
	
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
		{
			// Check all common session names
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
			
			for (const FName& SessionName : SessionNamesToCheck)
			{
				if (FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName))
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Destroying session '%s'"), *SessionName.ToString());
					
					// End session first if it's started
					if (Session->SessionState == EOnlineSessionState::InProgress)
					{
						SessionInterface->EndSession(SessionName);
					}
					
					// Destroy the session
					SessionInterface->DestroySession(SessionName);
					DestroyedCount++;
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Destroyed %d session(s)"), DestroyedCount);
	return DestroyedCount;
}

void UEOSKitGameInstanceSubsystem::OnStandaloneGameShutdown()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: STANDALONE GAME SHUTTING DOWN"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Cleaning up all sessions..."));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Destroy all active sessions
	int32 DestroyedCount = DestroyAllActiveSessions();
	
#if WITH_EOS_SDK
	// Also destroy EOS SDK sessions directly
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>())
		{
			EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
			if (PlatformHandle)
			{
				EOS_HSessions SessionsHandle = EOS_Platform_GetSessionsInterface(PlatformHandle);
				
				if (SessionsHandle)
				{
					// Try common session names that might be active
					const char* SessionNamesToCheck[] = {
						"Sessions",
						"Modified_EOS_Session",
						"Modified_EOS_Lobby",
						"test",
						"GameSession",
						"PartySession",
						"Lobby"
					};
					
					int32 SDKSessionsDestroyed = 0;
					
					for (const char* SessionName : SessionNamesToCheck)
					{
						EOS_Sessions_CopyActiveSessionHandleOptions CopyOptions = {};
						CopyOptions.ApiVersion = EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST;
						CopyOptions.SessionName = SessionName;
						
						EOS_HActiveSession ActiveSessionHandle = nullptr;
						EOS_EResult CopyResult = EOS_Sessions_CopyActiveSessionHandle(SessionsHandle, &CopyOptions, &ActiveSessionHandle);
						
						if (CopyResult == EOS_EResult::EOS_Success && ActiveSessionHandle)
						{
							UE_LOG(LogTemp, Warning, TEXT("EOSKit: Found active SDK session '%s', destroying..."), UTF8_TO_TCHAR(SessionName));
							
							// Destroy the session
							EOS_Sessions_DestroySessionOptions DestroyOptions = {};
							DestroyOptions.ApiVersion = EOS_SESSIONS_DESTROYSESSION_API_LATEST;
							DestroyOptions.SessionName = SessionName;
							
							EOS_Sessions_DestroySession(SessionsHandle, &DestroyOptions, nullptr,
								[](const EOS_Sessions_DestroySessionCallbackInfo* Data)
								{
									if (Data->ResultCode == EOS_EResult::EOS_Success)
									{
										UE_LOG(LogTemp, Log, TEXT("EOSKit: ✅ Destroyed SDK session"));
									}
								});
							
							EOS_ActiveSession_Release(ActiveSessionHandle);
							SDKSessionsDestroyed++;
						}
					}
					
					if (SDKSessionsDestroyed > 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Destroyed %d SDK session(s)"), SDKSessionsDestroyed);
					}
				}
			}
		}
	}
#endif
	
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Standalone game cleanup complete"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Game can now safely close"));
}
