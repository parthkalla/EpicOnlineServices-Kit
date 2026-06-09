// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_SetPresence_AsyncFunction.h"
#include "IOnlineSubsystemEOS.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/OnlineSessionNames.h"
#endif
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"

void UEOK_SetPresence_AsyncFunction::Activate()
{
	SetPresence();
	Super::Activate();
}


void UEOK_SetPresence_AsyncFunction::SetPresence()
{
	if (IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld()))
	{
		if(IOnlineIdentityPtr Identity = SubsystemRef->GetIdentityInterface())
		{
			if(IOnlinePresencePtr Presence = SubsystemRef->GetPresenceInterface())
			{
				if(!Identity->GetUniquePlayerId(0).IsValid())
				{
					UE_LOG(LogEOK, Error, TEXT("Identity is not valid to set presence"));
					OnFaliure.Broadcast("", PresenceStatus);
					SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
					MarkAsGarbage();
#else
					MarkPendingKill();
#endif
					return;
				}
				FOnlineUserPresenceStatus Status;
				switch (PresenceStatus)
				{
				case EPresenceStatus::PR_Online:
					Status.State = EOnlinePresenceState::Online;
					break;
				case EPresenceStatus::PR_Offline:
					break;
				case EPresenceStatus::PR_Away:
					Status.State = EOnlinePresenceState::Away;
					break;
				case EPresenceStatus::PR_ExtendedAway:
					Status.State = EOnlinePresenceState::ExtendedAway;
					break;
				case EPresenceStatus::PR_DoNotDisturb:
					Status.State = EOnlinePresenceState::DoNotDisturb;
					break;
				}
				FString RichPresenceStatus = RichPresence;
				Status.StatusStr = RichPresenceStatus;
				UE_LOG(LogEOK, Log, TEXT("Setting Presence to %s"), *RichPresenceStatus);
		
				Presence->SetPresence(*Identity->GetUniquePlayerId(0).Get(), Status, IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateUObject(this, &UEOK_SetPresence_AsyncFunction::OnSetPresenceCompleted));
				return;
			}
		}
	}
	OnFaliure.Broadcast("", PresenceStatus);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_SetPresence_AsyncFunction::OnSetPresenceCompleted(const class FUniqueNetId& UserId, const bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		OnSuccess.Broadcast(RichPresence, PresenceStatus);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
	else
	{
		OnFaliure.Broadcast("", PresenceStatus);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}

UEOK_SetPresence_AsyncFunction* UEOK_SetPresence_AsyncFunction::SetEOSPresence(FString RichPresense, EPresenceStatus PresenceStatus)
{
	UEOK_SetPresence_AsyncFunction* UEOK_SetPresenceObject = NewObject< UEOK_SetPresence_AsyncFunction>();
	UEOK_SetPresenceObject->RichPresence = RichPresense;
	UEOK_SetPresenceObject->PresenceStatus = PresenceStatus;
	return UEOK_SetPresenceObject;
}