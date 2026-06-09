// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_GetLeaderboards_AsyncFunction.h"

#include "OnlineSubsystemUtils.h"

UEOK_GetLeaderboards_AsyncFunction* UEOK_GetLeaderboards_AsyncFunction::GetLeaderboard(const FName LeaderboardName,
                                                                                       const int32 Range, const int32 AroundRank)
{
	UEOK_GetLeaderboards_AsyncFunction* BlueprintNode = NewObject<UEOK_GetLeaderboards_AsyncFunction>();
	BlueprintNode->LeaderboardName = LeaderboardName;
	BlueprintNode->Range = Range;
	BlueprintNode->AroundRank = AroundRank;
	return BlueprintNode;
}

void UEOK_GetLeaderboards_AsyncFunction::Activate()
{
	GetLeaderboardLocal();
	Super::Activate();
}

void UEOK_GetLeaderboards_AsyncFunction::GetLeaderboardLocal()
{
	if(const IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(GetWorld()))
	{
		if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if(const IOnlineLeaderboardsPtr LeaderboardsPointerRef = SubsystemRef->GetLeaderboardsInterface())
			{
				if(IdentityPointerRef->GetLoginStatus(0) != ELoginStatus::LoggedIn)
				{
					if(!bDelegateCalled)
					{
						OnFail.Broadcast(TArray<FEOKLeaderboardValue>());
						bDelegateCalled = true;
						SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						MarkAsGarbage();
#else
						MarkPendingKill();
#endif
					}
					return;
				}
				TArray<TSharedRef<const FUniqueNetId>> Usersvar;
				Usersvar.Add(IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef());
				FOnlineLeaderboardReadRef LeaderboardRead = MakeShareable(new FOnlineLeaderboardRead());
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
				FString LeaderboardNameString = LeaderboardName.ToString();
				LeaderboardRead->LeaderboardName = LeaderboardNameString;
#else
				LeaderboardRead->LeaderboardName = LeaderboardName;
#endif
				LeaderboardsPointerRef->OnLeaderboardReadCompleteDelegates.AddUObject(this,&UEOK_GetLeaderboards_AsyncFunction::OnGetLeaderboardCompleted,LeaderboardRead);
				if(!LeaderboardsPointerRef->ReadLeaderboardsAroundRank(AroundRank,Range,LeaderboardRead))
				{
					if(!bDelegateCalled)
					{
						OnFail.Broadcast(TArray<FEOKLeaderboardValue>());
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
					OnFail.Broadcast(TArray<FEOKLeaderboardValue>());
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
				OnFail.Broadcast(TArray<FEOKLeaderboardValue>());
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
			OnFail.Broadcast(TArray<FEOKLeaderboardValue>());
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

void UEOK_GetLeaderboards_AsyncFunction::OnGetLeaderboardCompleted(bool bWasSuccessful,
	FOnlineLeaderboardReadRef LeaderboardRead)
{
	if(bWasSuccessful)
	{
		TArray<FEOKLeaderboardValue> Result;

		for (auto Row : LeaderboardRead->Rows)
		{
			int32 Score = -1;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
			if (Row.Columns.Num() > 0)
			{
				const FString& FirstKey = Row.Columns.begin()->Key;
				Row.Columns.Find(FirstKey)->GetValue(Score);
			}
#else
			Row.Columns.Find("None")->GetValue(Score);
#endif
			FEOKLeaderboardValue LocalRow;
			LocalRow.Rank = Row.Rank;
			LocalRow.Score = Score;
			LocalRow.NickName = Row.NickName;
			Result.Add(LocalRow);
		}
		if(!bDelegateCalled)
		{
			OnSuccess.Broadcast(Result);
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
			OnFail.Broadcast(TArray<FEOKLeaderboardValue>());
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
