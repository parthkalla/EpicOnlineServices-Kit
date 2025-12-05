// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitEngineSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Engine/Engine.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#if WITH_EOS_SDK
	#include "eos_sdk.h"
	#include "eos_sessions.h"
	#include "eos_sessions_types.h"
	#include "eos_common.h"
#endif

#if WITH_EDITOR
	#include "Editor.h"
#endif

void UEOSKitEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Engine Subsystem Initialized"));
	
#if WITH_EDITOR
	// Register PIE delegates to handle EOS initialization/cleanup
	if (GEditor)
	{
		FEditorDelegates::BeginPIE.AddUObject(this, &UEOSKitEngineSubsystem::BeginPIE);
		FEditorDelegates::EndPIE.AddUObject(this, &UEOSKitEngineSubsystem::EndPIE);
		
		UE_LOG(LogTemp, Log, TEXT("EOSKit: PIE delegates registered"));
	}
#endif

	// Register application exit delegate (works in packaged game and PIE)
	OnExitDelegateHandle = FCoreDelegates::OnExit.AddUObject(this, &UEOSKitEngineSubsystem::OnApplicationWillTerminate);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Application exit delegate registered"));
}

void UEOSKitEngineSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Engine Subsystem Deinitialized"));
	
#if WITH_EDITOR
	// Unregister PIE delegates
	if (GEditor)
	{
		FEditorDelegates::BeginPIE.RemoveAll(this);
		FEditorDelegates::EndPIE.RemoveAll(this);
		
		UE_LOG(LogTemp, Log, TEXT("EOSKit: PIE delegates unregistered"));
	}
#endif

	// Unregister application exit delegate
	if (OnExitDelegateHandle.IsValid())
	{
		FCoreDelegates::OnExit.Remove(OnExitDelegateHandle);
		OnExitDelegateHandle.Reset();
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Application exit delegate unregistered"));
	}
	
	Super::Deinitialize();
}

void UEOSKitEngineSubsystem::BeginPIE(const bool bIsSimulating)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: PIE Session Starting"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Simulate Mode: %s"), bIsSimulating ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Preparing EOS Platform for PIE..."));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	bIsPIEActive = true;
	
	// Note: The EOSKit subsystem will be automatically initialized when the PIE world's game instance
	// gets created. We don't need to manually reload it here - the subsystem system handles this.
	
	// However, if you need to do any pre-PIE setup for EOS, you can do it here.
	// For example, you might want to ensure certain EOS services are ready, etc.
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: PIE session started - EOSKit will initialize with game instance"));
#endif
}

void UEOSKitEngineSubsystem::EndPIE(const bool bIsSimulating)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: PIE Session Ending"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Simulate Mode: %s"), bIsSimulating ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Cleaning up EOS sessions..."));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// CRITICAL: Destroy all EOS sessions before PIE ends
	// Otherwise they persist on the EOS backend and cause "already exists" errors
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
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
				if (FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName))
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Destroying session '%s' (State: %d) on PIE end"), 
						*SessionName.ToString(), (int32)Session->SessionState);
					
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
			
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Destroyed %d local session(s) on PIE end"), DestroyedCount);
		}
	}
	
#if WITH_EOS_SDK
	// CRITICAL: Destroy active EOS sessions directly via SDK
	// This catches any sessions created via SDK that aren't in the local session list
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
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
									UE_LOG(LogTemp, Warning, TEXT("EOSKit: Found active EOS SDK session '%s', destroying..."), UTF8_TO_TCHAR(SessionName));
									
									// Destroy the session
									EOS_Sessions_DestroySessionOptions DestroyOptions = {};
									DestroyOptions.ApiVersion = EOS_SESSIONS_DESTROYSESSION_API_LATEST;
									DestroyOptions.SessionName = SessionName;
									
									// Use static callback (lambdas can't capture in EOS SDK callbacks)
									EOS_Sessions_DestroySession(SessionsHandle, &DestroyOptions, nullptr,
										[](const EOS_Sessions_DestroySessionCallbackInfo* Data)
										{
											if (Data->ResultCode == EOS_EResult::EOS_Success)
											{
												UE_LOG(LogTemp, Log, TEXT("EOSKit: ✅ Destroyed an EOS SDK session"));
											}
											else
											{
												UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to destroy EOS SDK session: %s"), 
													UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
											}
										});
									
									EOS_ActiveSession_Release(ActiveSessionHandle);
									SDKSessionsDestroyed++;
								}
							}
							
							UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Checked and destroyed %d active EOS SDK session(s)"), SDKSessionsDestroyed);
						}
					}
				}
			}
		}
	}
#endif
	
	bIsPIEActive = false;
	UE_LOG(LogTemp, Log, TEXT("EOSKit: PIE session ended"));
#endif
}

void UEOSKitEngineSubsystem::OnApplicationWillTerminate()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: APPLICATION SHUTTING DOWN"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Terminating all EOS sessions..."));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Destroy all local sessions via OnlineSubsystem
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
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
			
			int32 DestroyedCount = 0;
			
			for (const FName& SessionName : SessionNamesToCheck)
			{
				if (FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName))
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Terminating session '%s' on app exit"), *SessionName.ToString());
					
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
			
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Terminated %d local session(s)"), DestroyedCount);
		}
	}
	
#if WITH_EOS_SDK
	// CRITICAL: Destroy active EOS sessions directly via SDK
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
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
									UE_LOG(LogTemp, Warning, TEXT("EOSKit: Found active EOS SDK session '%s', destroying on exit..."), UTF8_TO_TCHAR(SessionName));
									
									// Destroy the session
									EOS_Sessions_DestroySessionOptions DestroyOptions = {};
									DestroyOptions.ApiVersion = EOS_SESSIONS_DESTROYSESSION_API_LATEST;
									DestroyOptions.SessionName = SessionName;
									
									// Use static callback (synchronous for shutdown)
									EOS_Sessions_DestroySession(SessionsHandle, &DestroyOptions, nullptr,
										[](const EOS_Sessions_DestroySessionCallbackInfo* Data)
										{
											if (Data->ResultCode == EOS_EResult::EOS_Success)
											{
												UE_LOG(LogTemp, Log, TEXT("EOSKit: ✅ Destroyed EOS SDK session on exit"));
											}
										});
									
									EOS_ActiveSession_Release(ActiveSessionHandle);
									SDKSessionsDestroyed++;
								}
							}
							
							if (SDKSessionsDestroyed > 0)
							{
								UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Terminated %d EOS SDK session(s)"), SDKSessionsDestroyed);
							}
						}
					}
				}
			}
		}
	}
#endif
	
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ All sessions terminated on application exit"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Application can now safely close"));
}
