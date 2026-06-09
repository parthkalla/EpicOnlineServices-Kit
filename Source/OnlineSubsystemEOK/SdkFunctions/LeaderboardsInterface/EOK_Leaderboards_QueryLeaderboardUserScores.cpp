// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Leaderboards_QueryLeaderboardUserScores.h"

UEOK_Leaderboards_QueryLeaderboardUserScores* UEOK_Leaderboards_QueryLeaderboardUserScores::
EOK_Leaderboards_QueryLeaderboardUserScores(const FEOK_Leaderboards_QueryLeaderboardUserScoresOptions& Options)
{
	UEOK_Leaderboards_QueryLeaderboardUserScores* BlueprintNode = NewObject<UEOK_Leaderboards_QueryLeaderboardUserScores>();
	BlueprintNode->Var_Options = Options;
	return BlueprintNode;
}

void UEOK_Leaderboards_QueryLeaderboardUserScores::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_QueryLeaderboardUserScoresOptions Options = Var_Options.ToEOSLeaderboardsQueryLeaderboardUserScoresOptions();
			EOS_Leaderboards_QueryLeaderboardUserScores(EOSRef->LeaderboardsHandle, &Options, this, &UEOK_Leaderboards_QueryLeaderboardUserScores::Internal_OnQueryLeaderboardUserScoresCompleteCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query leaderboard user scores either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Leaderboards_QueryLeaderboardUserScores::Internal_OnQueryLeaderboardUserScoresCompleteCallback(
	const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo* Data)
{
	if(UEOK_Leaderboards_QueryLeaderboardUserScores* CallbackObj = static_cast<UEOK_Leaderboards_QueryLeaderboardUserScores*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [CallbackObj, Data]()
		{
			CallbackObj->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode));
			CallbackObj->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
CallbackObj->MarkAsGarbage();
#else
CallbackObj->MarkPendingKill();
#endif
		});
	}
}
