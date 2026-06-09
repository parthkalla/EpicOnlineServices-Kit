// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Stats_IngestStat.h"

#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Stats_IngestStat* UEOK_Stats_IngestStat::EOK_Stats_IngestStat(FEOK_ProductUserId LocalUserId,
                                                                   const TArray<FEOK_Stats_IngestData>& Stats, const FEOK_ProductUserId& TargetUserId)
{
	UEOK_Stats_IngestStat* Node = NewObject<UEOK_Stats_IngestStat>();
	Node->Var_Stats = Stats;
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Stats_IngestStat::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Stats_IngestStatOptions Options;
			Options.ApiVersion = EOS_STATS_INGESTSTAT_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			Options.StatsCount = Var_Stats.Num();
			EOS_Stats_IngestData* TempStats = new EOS_Stats_IngestData[Var_Stats.Num()];
			for (int i = 0; i < Var_Stats.Num(); i++)
			{
				TempStats[i] = Var_Stats[i].ToEOSStatsIngestData();
			}
			Options.Stats = TempStats;
			EOS_Stats_IngestStat(EOSRef->StatsHandle, &Options, this, &UEOK_Stats_IngestStat::Internal_OnStatsIngestStatComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_Stats_IngestStat::Activate: Failed to get EOS subsystem"));
	OnCallback.Broadcast(Var_LocalUserId, EEOK_Result::EOS_NotFound, Var_TargetUserId);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Stats_IngestStat::Internal_OnStatsIngestStatComplete(const EOS_Stats_IngestStatCompleteCallbackInfo* Data)
{
	if(UEOK_Stats_IngestStat* Node = static_cast<UEOK_Stats_IngestStat*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(Node->Var_LocalUserId, static_cast<EEOK_Result>(Data->ResultCode), Node->Var_TargetUserId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
