// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_GetAchievementDetails_AsyncFunction.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"

UEOK_GetAchievementDetails_AsyncFunction* UEOK_GetAchievementDetails_AsyncFunction::GetEOKAchievementDescription(FEOK_Achievement Achievement)	
{
	UEOK_GetAchievementDetails_AsyncFunction* UEOK_GetAchievementDescriptionObject = NewObject<UEOK_GetAchievementDetails_AsyncFunction>();
	UEOK_GetAchievementDescriptionObject->Var_Achievement = Achievement;
	return UEOK_GetAchievementDescriptionObject;
}

void UEOK_GetAchievementDetails_AsyncFunction::Activate()
{
	GetAchievementDescription();
	Super::Activate();
}

void UEOK_GetAchievementDetails_AsyncFunction::GetAchievementDescription()
{
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get("EOK"))
	{
		if(const IOnlineAchievementsPtr AchievementsPtrRef = SubsystemRef->GetAchievementsInterface())
		{
			if(const IOnlineIdentityPtr IdentityPtrRef = SubsystemRef->GetIdentityInterface())
			{
				AchievementsPtrRef->QueryAchievementDescriptions(*IdentityPtrRef->GetUniquePlayerId(0), FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &UEOK_GetAchievementDetails_AsyncFunction::OnAchievementDescriptionCompleted));
			}
			else
			{
				if(!bDelegateCalled)
				{
					OnFail.Broadcast(FEOK_AchievementDescription());
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
			if(!bDelegateCalled)
			{
				OnFail.Broadcast(FEOK_AchievementDescription());
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
		if(!bDelegateCalled)
		{
			OnFail.Broadcast(FEOK_AchievementDescription());
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
void UEOK_GetAchievementDetails_AsyncFunction::OnAchievementDescriptionCompleted(const FUniqueNetId& UniqueNetId,bool bWasSuccess)
{
	if(bWasSuccess)
	{
		if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get("EOK"))
		{
			if(const IOnlineAchievementsPtr AchievementsPtrRef = SubsystemRef->GetAchievementsInterface())
			{
				FEOK_AchievementDescription LocalAchievementDescription;
				FOnlineAchievementDesc CachedAchievements;
				AchievementsPtrRef->GetCachedAchievementDescription(Var_Achievement.Id, CachedAchievements);
				LocalAchievementDescription.Id = Var_Achievement.Id;
				LocalAchievementDescription.Title = CachedAchievements.Title;
				LocalAchievementDescription.Progress = Var_Achievement.Progress;
				LocalAchievementDescription.LockedDesc= CachedAchievements.LockedDesc;
				LocalAchievementDescription.bIsHidden = CachedAchievements.bIsHidden;
				LocalAchievementDescription.UnlockedDesc = CachedAchievements.UnlockedDesc;
				LocalAchievementDescription.UnlockTime = CachedAchievements.UnlockTime;
				OnSuccess.Broadcast(LocalAchievementDescription);
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
				return;
			}
		}
	}
	if(!bDelegateCalled)
	{
		OnFail.Broadcast(FEOK_AchievementDescription());
		bDelegateCalled = true;
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}
