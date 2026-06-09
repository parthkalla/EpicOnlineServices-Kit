// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_GetStats_AsyncFunction.h"

#include "OnlineError.h"
#include "OnlineStatsEOS.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"

UEOK_GetStats_AsyncFunction* UEOK_GetStats_AsyncFunction::GetStats(TArray<FString> StatName)
{
	UEOK_GetStats_AsyncFunction* BlueprintNode = NewObject<UEOK_GetStats_AsyncFunction>();
	BlueprintNode->StatName = StatName;
	return BlueprintNode;
}

void UEOK_GetStats_AsyncFunction::Activate()
{
	GetStats();
	Super::Activate();
}

void UEOK_GetStats_AsyncFunction::GetStats()
{
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get("EOK"))
	{
		if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if(const IOnlineStatsPtr StatsPointerRef = SubsystemRef->GetStatsInterface())
			{
				TArray<TSharedRef<const FUniqueNetId>> Usersvar;
				if(!IdentityPointerRef->GetUniquePlayerId(0))
				{
					if(!bDelegateCalled)
					{
						OnFail.Broadcast(TArray<FEOK_Stats>());
						bDelegateCalled = true;
						SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						MarkAsGarbage();
#else
						MarkPendingKill();
#endif
					}
				}
				Usersvar.Add(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef());
				StatsPointerRef->QueryStats(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef(),Usersvar,StatName,FOnlineStatsQueryUsersStatsComplete::CreateUObject(this, &UEOK_GetStats_AsyncFunction::OnGetStatsCompleted));
			}
			else
			{
				if(!bDelegateCalled)
				{
					OnFail.Broadcast(TArray<FEOK_Stats>());
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
				OnFail.Broadcast(TArray<FEOK_Stats>());
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
			OnFail.Broadcast(TArray<FEOK_Stats>());
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

void UEOK_GetStats_AsyncFunction::OnGetStatsCompleted(const FOnlineError& ResultState,
	const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult)
{
	if(ResultState.WasSuccessful())
	{
		TArray<FEOK_Stats> LocalStatsArray;
		for(const auto& StatsVar : UsersStatsResult)
		{
			for(auto StoredValueRef : StatsVar->Stats)
			{
				FString Keyname = StoredValueRef.Key;
				int32 Value;
				StoredValueRef.Value.GetValue(Value);
				FEOK_Stats LocalStats;
				LocalStats.StatsName = Keyname;
				LocalStats.StatsValue = FString::FromInt(Value);
				LocalStatsArray.Add(LocalStats);
			}
		}
		if(!bDelegateCalled)
		{
			OnSuccess.Broadcast(LocalStatsArray);
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
			OnFail.Broadcast(TArray<FEOK_Stats>());
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
