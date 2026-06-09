// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_CreateLobby_AsyncFunction.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"

void UEOK_CreateLobby_AsyncFunction::Activate()
{
	CreateLobby();
	Super::Activate();
}

void UEOK_CreateLobby_AsyncFunction::CreateLobby()
{
	if(const IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(this->GetWorld(), "EOK"))
	{
		if(const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			FOnlineSessionSettings SessionCreationInfo;
			SessionCreationInfo.bIsDedicated = false;
			SessionCreationInfo.bAllowInvites = Var_CreateLobbySettings.bAllowInvites;
			SessionCreationInfo.bIsLANMatch = Var_CreateLobbySettings.bIsLanMatch;
			SessionCreationInfo.NumPublicConnections = NumberOfPublicConnections;
			SessionCreationInfo.NumPrivateConnections = Var_CreateLobbySettings.NumberOfPrivateConnections;
			SessionCreationInfo.bUseLobbiesIfAvailable = true;
			SessionCreationInfo.bUseLobbiesVoiceChatIfAvailable = Var_CreateLobbySettings.bUseVoiceChat;
			SessionCreationInfo.bUsesPresence = Var_CreateLobbySettings.bUsePresence;
			SessionCreationInfo.bAllowJoinViaPresence = Var_CreateLobbySettings.bUsePresence;
			SessionCreationInfo.bAllowJoinViaPresenceFriendsOnly = Var_CreateLobbySettings.bUsePresence;
			SessionCreationInfo.bShouldAdvertise = Var_CreateLobbySettings.bShouldAdvertise;
			SessionCreationInfo.bAllowJoinInProgress = Var_CreateLobbySettings.bAllowJoinInProgress;
			if(MemberSettings.Num() > 0)
			{
				TUniqueNetIdMap<FSessionSettings> LocalMemberSettings;
				FSessionSettings LocalSessionSettings;
				for(auto& MemberSetting : MemberSettings)
				{
					LocalSessionSettings.Add(FName(*MemberSetting.Key), MemberSetting.Value.GetVariantData());
				}
				LocalMemberSettings.Add(SubsystemRef->GetIdentityInterface()->GetUniquePlayerId(0).ToSharedRef(), LocalSessionSettings);
			}
#if ENGINE_MAJOR_VERSION == 5
			SessionCreationInfo.SessionIdOverride = Var_CreateLobbySettings.LobbyIDOverride;
			SessionCreationInfo.Set(SETTING_HOST_MIGRATION, Var_CreateLobbySettings.bSupportHostMigration, EOnlineDataAdvertisementType::ViaOnlineService);
#else
			SessionCreationInfo.Set("SETTING_SESSION_ID_OVERRIDE", Var_CreateLobbySettings.LobbyIDOverride, EOnlineDataAdvertisementType::ViaOnlineService);
			SessionCreationInfo.Set("SETTING_HOST_MIGRATION", Var_CreateLobbySettings.bSupportHostMigration, EOnlineDataAdvertisementType::ViaOnlineService);
#endif
			{
				FOnlineSessionSetting LocalVNameSetting;
				LocalVNameSetting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				LocalVNameSetting.Data = *VSessionName.ToString();
				SessionCreationInfo.Set(FName(TEXT("SessionName")), LocalVNameSetting);
			}
			{
				FOnlineSessionSetting bPartySession;
				bPartySession.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				bPartySession.Data = false;
				SessionCreationInfo.Set(FName(TEXT("IsPartySession")), bPartySession);
			}
			for (auto& Settings_SingleValue : SessionSettings)
			{
				if (Settings_SingleValue.Key.Len() == 0)
				{
					continue;
				}
				FOnlineSessionSetting Setting;
				Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				Setting.Data = Settings_SingleValue.Value.GetVariantData();
				SessionCreationInfo.Set(FName(*Settings_SingleValue.Key), Setting);
			}
			SessionPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOK_CreateLobby_AsyncFunction::OnCreateLobbyCompleted);
			SessionPtrRef->CreateSession(0,VSessionName,SessionCreationInfo);
		}
		else
		{
			if(bDelegateCalled == false)
			{
				UE_LOG(LogOnline, Warning, TEXT("EOK: SessionPtrRef is null"));
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
			UE_LOG(LogOnline, Warning, TEXT("EOK: SubsystemRef is null"));
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

void UEOK_CreateLobby_AsyncFunction::OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if(bDelegateCalled == false)
		{
			const IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
			if(const FOnlineSession* CurrentSession = Sessions->GetNamedSession(VSessionName))
			{
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
			UE_LOG(LogOnline, Warning, TEXT("EOK: CreateLobby failed and response was false. Check logs for more information."));
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

UEOK_CreateLobby_AsyncFunction* UEOK_CreateLobby_AsyncFunction::CreateEOKLobby(TMap<FString, FEOKAttribute> SessionSettings, TMap<FString, FEOKAttribute> MemberSettings,
FName SessionName,
	int32 NumberOfPublicConnections, FCreateLobbySettings ExtraSettings)
{
	UEOK_CreateLobby_AsyncFunction* UEOK_CreateLobbyObject= NewObject<UEOK_CreateLobby_AsyncFunction>();
	UEOK_CreateLobbyObject->NumberOfPublicConnections = NumberOfPublicConnections;
	UEOK_CreateLobbyObject->SessionSettings = SessionSettings;
	UEOK_CreateLobbyObject->MemberSettings = MemberSettings;
	UEOK_CreateLobbyObject->Var_CreateLobbySettings = ExtraSettings;
	UEOK_CreateLobbyObject->VSessionName = SessionName;
	return UEOK_CreateLobbyObject;
}
