// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Leaderboards_QueryLeaderboardDefinitions.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Leaderboards_QueryLeaderboardDefinitions* UEOK_Leaderboards_QueryLeaderboardDefinitions::
EOK_Leaderboards_QueryLeaderboardDefinitions(const FEOK_Leaderboards_QueryLeaderboardDefinitionsOptions& Options)
{
	UEOK_Leaderboards_QueryLeaderboardDefinitions* BlueprintNode = NewObject<UEOK_Leaderboards_QueryLeaderboardDefinitions>();
	BlueprintNode->Var_Options = Options;
	return BlueprintNode;
}

void UEOK_Leaderboards_QueryLeaderboardDefinitions::OnQueryLeaderboardDefinitionsCompleteCallback(
	const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo* Data)
{
	if(UEOK_Leaderboards_QueryLeaderboardDefinitions* CallbackObj = static_cast<UEOK_Leaderboards_QueryLeaderboardDefinitions*>(Data->ClientData))
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

void UEOK_Leaderboards_QueryLeaderboardDefinitions::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_QueryLeaderboardDefinitionsOptions Options = Var_Options.ToEOSLeaderboardsQueryLeaderboardDefinitionsOptions();
			EOS_Leaderboards_QueryLeaderboardDefinitions(EOSRef->LeaderboardsHandle, &Options, this, &UEOK_Leaderboards_QueryLeaderboardDefinitions::OnQueryLeaderboardDefinitionsCompleteCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query leaderboard definitions either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
