// Copyright (C) 2024, All Rights Reserved.

#include "EOSCreateEOKSessionAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

void UEOSCreateEOKSessionAsync::Activate()
{
	CreateSession();
	Super::Activate();
}

void UEOSCreateEOKSessionAsync::CreateSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Creating EOS Session via OnlineSubsystem (like EIK)"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Public Connections: %d"), NumberOfPublicConnections);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	if (!CachedWorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED - WorldContextObject is null"));
		OnFail.Broadcast(TEXT("WorldContextObject is null"));
		SetReadyToDestroy();
		return;
	}
	
	// Use OnlineSubsystem interface (like EIK does)
	if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			// Build session settings
			FOnlineSessionSettings Settings;
			Settings.NumPublicConnections = NumberOfPublicConnections;
			Settings.NumPrivateConnections = ExtraSettings.NumberOfPrivateConnections;
			Settings.bShouldAdvertise = ExtraSettings.bShouldAdvertise;
			Settings.bUsesPresence = ExtraSettings.bUsePresence;
			Settings.bAllowJoinInProgress = ExtraSettings.bAllowJoinInProgress;
			Settings.bAllowJoinViaPresence = ExtraSettings.bAllowJoinViaPresence;
			Settings.bAllowJoinViaPresenceFriendsOnly = ExtraSettings.bAllowJoinViaPresenceFriendsOnly;
			Settings.bIsLANMatch = false; // Always false for EOS
			Settings.bUseLobbiesIfAvailable = false;
			Settings.bAllowInvites = true;
			Settings.bIsDedicated = DedicatedServerSettings.bUseDedicatedServer;
			
			// Add BucketId
			if (!ExtraSettings.BucketId.IsEmpty())
			{
				Settings.Set(FName(TEXT("BucketId")), ExtraSettings.BucketId, EOnlineDataAdvertisementType::ViaOnlineService);
			}
			
			// Add custom attributes
			for (const auto& Setting : SessionSettings)
			{
				if (Setting.Key.IsEmpty()) continue;
				
				FOnlineSessionSetting SessionSetting;
				SessionSetting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				
				if (!Setting.Value.StringValue.IsEmpty())
				{
					SessionSetting.Data.SetValue(Setting.Value.StringValue);
				}
				else if (Setting.Value.IntValue != 0)
				{
					SessionSetting.Data.SetValue(Setting.Value.IntValue);
				}
				else
				{
					SessionSetting.Data.SetValue(Setting.Value.BoolValue);
				}
				
				Settings.Set(FName(*Setting.Key), SessionSetting);
			}
			
			// Register callback
			SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSCreateEOKSessionAsync::OnCreateSessionCompleted);
			
			// Create session via OnlineSubsystem (creates local + EOS backend)
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Calling OnlineSubsystem->CreateSession()..."));
			SessionPtr->CreateSession(0, VSessionName, Settings);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED - SessionInterface is null"));
			OnFail.Broadcast(TEXT("SessionInterface is null"));
			SetReadyToDestroy();
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED - OnlineSubsystem is null"));
		OnFail.Broadcast(TEXT("OnlineSubsystem is null"));
		SetReadyToDestroy();
		return;
	}
}

void UEOSCreateEOKSessionAsync::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Session created successfully!"));
		
		// Get session ID for broadcast
		FString SessionId;
		if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
			{
				if (FNamedOnlineSession* Session = SessionPtr->GetNamedSession(SessionName))
				{
					if (Session->SessionInfo.IsValid())
					{
						SessionId = Session->SessionInfo->GetSessionId().ToString();
					}
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *SessionName.ToString());
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session ID: %s"), *SessionId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Note: Session is created but NOT started. Call StartSession separately if needed."));
				}
			}
		}
		
		// Broadcast success - session is created but NOT started
		OnSuccess.Broadcast(SessionId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create session via OnlineSubsystem"));
		OnFail.Broadcast(TEXT("CreateSession failed"));
	}
	
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

UEOSCreateEOKSessionAsync* UEOSCreateEOKSessionAsync::CreateEOKSession(
	UObject* WorldContextObject,
	TMap<FString, FEOSKitAttribute> SessionSettings,
	FName SessionName,
	int32 NumberOfPublicConnections,
	FEOSKitDedicatedServerSettings DedicatedServerSettings,
	FEOSKitCreateSessionSettings ExtraSettings)
{
	UEOSCreateEOKSessionAsync* Ueik_CreateSessionObject = NewObject<UEOSCreateEOKSessionAsync>();
	Ueik_CreateSessionObject->CachedWorldContextObject = WorldContextObject;
	Ueik_CreateSessionObject->SessionSettings = SessionSettings;
	Ueik_CreateSessionObject->NumberOfPublicConnections = NumberOfPublicConnections;
	Ueik_CreateSessionObject->DedicatedServerSettings = DedicatedServerSettings;
	Ueik_CreateSessionObject->VSessionName = SessionName;
	Ueik_CreateSessionObject->ExtraSettings = ExtraSettings;
	return Ueik_CreateSessionObject;
}
