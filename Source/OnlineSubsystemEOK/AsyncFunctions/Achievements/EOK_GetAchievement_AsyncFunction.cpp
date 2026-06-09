// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_GetAchievement_AsyncFunction.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"

UEOK_GetAchievement_AsyncFunction* UEOK_GetAchievement_AsyncFunction::GetEOKAchievements()
{
	UEOK_GetAchievement_AsyncFunction* UEOK_GetAchievementObject = NewObject<UEOK_GetAchievement_AsyncFunction>();
	return UEOK_GetAchievementObject;
}

void UEOK_GetAchievement_AsyncFunction::Activate()
{
	GetAchievements();
	Super::Activate();
}

void UEOK_GetAchievement_AsyncFunction::GetAchievements()
{
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get("EOK"))
	{
		if(const IOnlineAchievementsPtr AchievementsPtrRef = SubsystemRef->GetAchievementsInterface())
		{
			if(const IOnlineIdentityPtr IdentityPtrRef = SubsystemRef->GetIdentityInterface())
			{
				AchievementsPtrRef->QueryAchievements(*IdentityPtrRef->GetUniquePlayerId(0), FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &UEOK_GetAchievement_AsyncFunction::OnAchievementsCompleted));
			}
			else
			{
				if(!bDelegateCalled)
				{
					OnFail.Broadcast(TArray<FEOK_Achievement>());
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
				OnFail.Broadcast(TArray<FEOK_Achievement>());
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
			OnFail.Broadcast(TArray<FEOK_Achievement>());
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

void UEOK_GetAchievement_AsyncFunction::OnAchievementsCompleted(const FUniqueNetId& UniqueNetId, bool bWasSuccess)
{
	if(bWasSuccess)
	{
		if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get("EOK"))
		{
			if(const IOnlineAchievementsPtr AchievementsPtrRef = SubsystemRef->GetAchievementsInterface())
			{
				TArray<FEOK_Achievement> AchievementsArray;
				TArray<FOnlineAchievement> CachedAchievements;
				AchievementsPtrRef->GetCachedAchievements(UniqueNetId, CachedAchievements);
				for(auto Achievement : CachedAchievements)
				{
					FEOK_Achievement LocalAchievement;
					LocalAchievement.Id = Achievement.Id;
					LocalAchievement.Progress = Achievement.Progress;
					AchievementsArray.Add(LocalAchievement);
				}
				OnSuccess.Broadcast(AchievementsArray);
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
		OnFail.Broadcast(TArray<FEOK_Achievement>());
		bDelegateCalled = true;
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}