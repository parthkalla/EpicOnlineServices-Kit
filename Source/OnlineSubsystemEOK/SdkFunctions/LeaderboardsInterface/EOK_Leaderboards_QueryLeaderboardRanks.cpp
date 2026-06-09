// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Leaderboards_QueryLeaderboardRanks.h"

UEOK_Leaderboards_QueryLeaderboardRanks* UEOK_Leaderboards_QueryLeaderboardRanks::EOK_Leaderboards_QueryLeaderboardRanks(const FEOK_Leaderboards_QueryLeaderboardRanksOptions& Options)
{
	UEOK_Leaderboards_QueryLeaderboardRanks* BlueprintNode = NewObject<UEOK_Leaderboards_QueryLeaderboardRanks>();
	BlueprintNode->Var_Options = Options;
	return BlueprintNode;
}

void UEOK_Leaderboards_QueryLeaderboardRanks::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_QueryLeaderboardRanksOptions Options = Var_Options.ToEOSLeaderboardsQueryLeaderboardRanksOptions();
			EOS_Leaderboards_QueryLeaderboardRanks(EOSRef->LeaderboardsHandle, &Options, this, &UEOK_Leaderboards_QueryLeaderboardRanks::Internal_OnQueryLeaderboardRanksCompleteCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query leaderboard ranks either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, "");
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Leaderboards_QueryLeaderboardRanks::Internal_OnQueryLeaderboardRanksCompleteCallback(
	const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo* Data)
{
	if(UEOK_Leaderboards_QueryLeaderboardRanks* CallbackObj = static_cast<UEOK_Leaderboards_QueryLeaderboardRanks*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [CallbackObj, Data]()
		{
			CallbackObj->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LeaderboardId);
			CallbackObj->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
CallbackObj->MarkAsGarbage();
#else
CallbackObj->MarkPendingKill();
#endif
		});
	}
}