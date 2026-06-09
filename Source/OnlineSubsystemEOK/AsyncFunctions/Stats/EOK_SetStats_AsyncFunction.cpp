// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_SetStats_AsyncFunction.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineStatsInterface.h"

UEOK_SetStats_AsyncFunction* UEOK_SetStats_AsyncFunction::SetEOKStats(const FString& StatName, const int32 StatValue)
{
	UEOK_SetStats_AsyncFunction* BlueprintNode = NewObject<UEOK_SetStats_AsyncFunction>();
	BlueprintNode->StatName = StatName;
	BlueprintNode->StatValue = StatValue;
	return BlueprintNode;
}

void UEOK_SetStats_AsyncFunction::OnEUpdateStatsCompleted(const FOnlineError& Result)
{
	if(Result.WasSuccessful() || Result.bSucceeded)
	{
		if(!bDelegateCalled)
		{
			OnSuccess.Broadcast();
			bDelegateCalled = true;
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	else
	{
		if(!bDelegateCalled)
		{
			OnFail.Broadcast();
			bDelegateCalled = true;
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	return;
}

void UEOK_SetStats_AsyncFunction::Activate()
{
	SetEOKStatsLocal();
	Super::Activate();
}

void UEOK_SetStats_AsyncFunction::SetEOKStatsLocal()
{
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get())
	{
		if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if(const IOnlineStatsPtr StatsPointerRef = SubsystemRef->GetStatsInterface())
			{
				FOnlineStatsUserUpdatedStats StatVar = FOnlineStatsUserUpdatedStats(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef());
				StatVar.Stats.Add(StatName, FOnlineStatUpdate(StatValue,FOnlineStatUpdate::EOnlineStatModificationType::Sum));
				TArray<FOnlineStatsUserUpdatedStats> StatArray;
				StatArray.Add(StatVar);
				StatsPointerRef->UpdateStats(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef(),StatArray,FOnlineStatsUpdateStatsComplete::CreateUObject(this, &UEOK_SetStats_AsyncFunction::OnEUpdateStatsCompleted));
			}
			else
			{
				if(!bDelegateCalled)
				{
					OnFail.Broadcast();
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
				OnFail.Broadcast();
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
			OnFail.Broadcast();
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
