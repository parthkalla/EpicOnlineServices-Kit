// Copyright (C) 2024, All Rights Reserved.

#include "EOSCreateEOKSessionAsync.h"
#include "CoreGlobals.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "UObject/Package.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/OnlineSessionNames.h"
#endif

void UEOSCreateEOKSessionAsync::Activate()
{
	CreateSession();
	Super::Activate();
}

void UEOSCreateEOKSessionAsync::CreateSession()
{
	if(IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(this->GetWorld(), "EOSKit"))
	{
		if(IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			FOnlineSessionSettings SessionCreationInfo;
			SessionCreationInfo.bIsDedicated = DedicatedServerSettings.bUseDedicatedServer;
			SessionCreationInfo.bUsesPresence = ExtraSettings.bUsePresence;
			SessionCreationInfo.bAllowJoinViaPresence = ExtraSettings.bAllowJoinViaPresence;
			SessionCreationInfo.bAllowJoinViaPresenceFriendsOnly = ExtraSettings.bAllowJoinViaPresenceFriendsOnly;
			SessionCreationInfo.bAllowInvites = true;
			if(DedicatedServerSettings.bUseDedicatedServer)
			{
				SessionCreationInfo.bUsesPresence = false;
				SessionCreationInfo.bAllowJoinViaPresence = false;
				SessionCreationInfo.bAllowJoinViaPresenceFriendsOnly = false;
				SessionCreationInfo.bAllowInvites = false;
			}
			SessionCreationInfo.bIsLANMatch = ExtraSettings.bIsLanMatch;
			SessionCreationInfo.NumPublicConnections = NumberOfPublicConnections;
			SessionCreationInfo.NumPrivateConnections = ExtraSettings.NumberOfPrivateConnections;
			SessionCreationInfo.bUseLobbiesIfAvailable = false;
			SessionCreationInfo.bUseLobbiesVoiceChatIfAvailable = false;
			SessionCreationInfo.bShouldAdvertise = ExtraSettings.bShouldAdvertise;
			SessionCreationInfo.bAllowJoinInProgress = ExtraSettings.bAllowJoinInProgress;
			{
				FOnlineSessionSetting LocalVNameSetting;
				LocalVNameSetting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				LocalVNameSetting.Data = *VSessionName.ToString();
				SessionCreationInfo.Set(FName(TEXT("SessionName")), LocalVNameSetting);
			}
			{
				FOnlineSessionSetting LocalbEnforceSanctions;
				LocalbEnforceSanctions.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				LocalbEnforceSanctions.Data = ExtraSettings.bEnforceSanctions;
				SessionCreationInfo.Set(FName(TEXT("SANCTIONENABLED")), LocalbEnforceSanctions);
			}
			{
				FOnlineSessionSetting bPartySession;
				bPartySession.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				bPartySession.Data = false;
				SessionCreationInfo.Set(FName(TEXT("IsPartySession")), bPartySession);
			}
			SessionCreationInfo.Settings.Add( FName(TEXT("REGIONINFO")), FOnlineSessionSetting(UEnum::GetValueAsString(ExtraSettings.Region), EOnlineDataAdvertisementType::ViaOnlineService));
			
			// CRITICAL: Store the current map path so clients know which map to load
			// Automatically fetch from engine - host is already on the session map
			if (UWorld* World = GetWorld())
			{
				FString MapPath;
				
				// Get the full map path (e.g., /Game/ThirdPerson/Maps/ThirdPersonMap)
				// Try multiple methods to get the map path
				if (ULevel* PersistentLevel = World->GetCurrentLevel())
				{
					if (UPackage* LevelPackage = PersistentLevel->GetOutermost())
					{
						MapPath = LevelPackage->GetName();
						
						// Remove the "UEDPIE_0_" prefix if present (PIE adds this)
						MapPath.RemoveFromStart(TEXT("UEDPIE_0_"));
						MapPath.RemoveFromStart(TEXT("UEDPIE_"));
						
						// Remove the "_PersistentLevel" suffix if present
						MapPath.RemoveFromEnd(TEXT("_PersistentLevel"));
					}
				}
				
				// Fallback: Use GetMapName() and try to construct path
				if (MapPath.IsEmpty())
				{
					FString MapName = World->GetMapName();
					if (!MapName.IsEmpty())
					{
						// Remove PIE prefixes
						MapName.RemoveFromStart(TEXT("UEDPIE_0_"));
						MapName.RemoveFromStart(TEXT("UEDPIE_"));
						
						// Try to get the package path
						if (UPackage* WorldPackage = World->GetOutermost())
						{
							MapPath = WorldPackage->GetName();
							MapPath.RemoveFromStart(TEXT("UEDPIE_0_"));
							MapPath.RemoveFromStart(TEXT("UEDPIE_"));
						}
						else
						{
							// Last resort: use map name as-is
							MapPath = MapName;
						}
					}
				}
				
				if (!MapPath.IsEmpty())
				{
					// Store as "MapName" setting (matches EIK convention)
					SessionCreationInfo.Settings.Add(FName(TEXT("MapName")), FOnlineSessionSetting(MapPath, EOnlineDataAdvertisementType::ViaOnlineService));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Automatically stored map path from engine: %s"), *MapPath);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ⚠️ Could not determine map path from world"));
				}
			}
			
			if(DedicatedServerSettings.bUseDedicatedServer)
			{
				FString Port = FString::FromInt(DedicatedServerSettings.ServerPort);				
				SessionCreationInfo.Settings.Add( FName(TEXT("PortInfo")), FOnlineSessionSetting(Port, EOnlineDataAdvertisementType::ViaOnlineService));
				SessionCreationInfo.Settings.Add( FName(TEXT("IsDedicatedServer")), FOnlineSessionSetting(true, EOnlineDataAdvertisementType::ViaOnlineService));
			}
			for (auto& Settings_SingleValue : SessionSettings)
			{
				if (Settings_SingleValue.Key.Len() == 0)
				{
					continue;
				}

				FOnlineSessionSetting Setting;
				Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				
				// Convert FEOSKitAttribute to FVariantData (like EIK does with FEIKAttribute.GetVariantData())
				if (!Settings_SingleValue.Value.StringValue.IsEmpty())
				{
					Setting.Data.SetValue(Settings_SingleValue.Value.StringValue);
				}
				else if (Settings_SingleValue.Value.IntValue != 0)
				{
					Setting.Data.SetValue(Settings_SingleValue.Value.IntValue);
				}
				else if (Settings_SingleValue.Value.FloatValue != 0.0f)
				{
					Setting.Data.SetValue(Settings_SingleValue.Value.FloatValue);
				}
				else
				{
					Setting.Data.SetValue(Settings_SingleValue.Value.BoolValue);
				}
				
				SessionCreationInfo.Set(FName(*Settings_SingleValue.Key), Setting);
			}
			SessionPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSCreateEOKSessionAsync::OnCreateSessionCompleted);
			SessionPtrRef->CreateSession(0,VSessionName,SessionCreationInfo);
		}
		else
		{
			if(bDelegateCalled == false)
			{
				OnFail.Broadcast("");
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
				bDelegateCalled = true;
			}
		}
	}
	else
	{
		if(bDelegateCalled == false)
		{
			OnFail.Broadcast("");
			bDelegateCalled = true;
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}

void UEOSCreateEOKSessionAsync::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if(bDelegateCalled == false)
		{
			const IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
			if(const FOnlineSession* CurrentSession = Sessions->GetNamedSession(VSessionName))
			{
				// CRITICAL FIX: Start the session immediately after creation
				// Without this, the session remains in Pending state and is not discoverable/joinable
				Sessions->OnStartSessionCompleteDelegates.AddUObject(this, &UEOSCreateEOKSessionAsync::OnStartSessionCompleted);
				bool bStartResult = Sessions->StartSession(VSessionName);
				if (!bStartResult)
				{
					UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to start session '%s'"), *VSessionName.ToString());
					OnFail.Broadcast(TEXT("StartSession failed"));
					bDelegateCalled = true;
					SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
					MarkAsGarbage();
#else
					MarkPendingKill();
#endif
				}
				// Success will be broadcast in OnStartSessionCompleted callback
				// Don't broadcast here - wait for StartSession to complete
			}
			else
			{
				OnSuccess.Broadcast("");
				bDelegateCalled = true;
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
			}
		}
	}
	else
	{
		if(bDelegateCalled == false)
		{
			OnFail.Broadcast("");
			bDelegateCalled = true;
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}

void UEOSCreateEOKSessionAsync::OnStartSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	if(bWasSuccessful && !bDelegateCalled)
	{
		const IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
		if(const FOnlineSession* CurrentSession = Sessions->GetNamedSession(VSessionName))
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Session created AND started successfully! SessionID: %s"), 
				*CurrentSession->SessionInfo.Get()->GetSessionId().ToString());
			OnSuccess.Broadcast(CurrentSession->SessionInfo.Get()->GetSessionId().ToString());
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
			UE_LOG(LogTemp, Error, TEXT("EOSKit: StartSession succeeded but session not found"));
			OnFail.Broadcast(TEXT("Session not found after StartSession"));
			bDelegateCalled = true;
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	else if(!bDelegateCalled)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: StartSession failed for session '%s'"), *VSessionName.ToString());
		OnFail.Broadcast(TEXT("StartSession failed"));
		bDelegateCalled = true;
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}

UEOSCreateEOKSessionAsync* UEOSCreateEOKSessionAsync::CreateEOKSession(
	TMap<FString, FEOSKitAttribute> SessionSettings,
		FName SessionName,
		int32 NumberOfPublicConnections,
	FEOSKitDedicatedServerSettings DedicatedServerSettings, FEOSKitCreateSessionSettings ExtraSettings)
{
	UEOSCreateEOKSessionAsync* Ueik_CreateSessionObject= NewObject<UEOSCreateEOKSessionAsync>();
	Ueik_CreateSessionObject->SessionSettings = SessionSettings;
	Ueik_CreateSessionObject->NumberOfPublicConnections = NumberOfPublicConnections;
	Ueik_CreateSessionObject->DedicatedServerSettings = DedicatedServerSettings;
	Ueik_CreateSessionObject->VSessionName = SessionName;
	Ueik_CreateSessionObject->ExtraSettings = ExtraSettings;
	return Ueik_CreateSessionObject;
}
