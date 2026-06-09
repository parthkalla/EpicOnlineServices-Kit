// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Stats_QueryStats.h"

#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Stats_QueryStats* UEOK_Stats_QueryStats::EOK_Stats_QueryStats(FEOK_ProductUserId LocalUserId,
                                                                   const FEOK_ProductUserId& TargetUserId, int64 StartTime, int64 EndTime, const TArray<FString>& StatNames)
{
	UEOK_Stats_QueryStats* Node = NewObject<UEOK_Stats_QueryStats>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	Node->Var_StartTime = StartTime;
	Node->Var_EndTime = EndTime;
	Node->Var_StatNames = StatNames;
	return Node;
}

void UEOK_Stats_QueryStats::Internal_OnStatsQueryStatsComplete(const EOS_Stats_OnQueryStatsCompleteCallbackInfo* Data)
{
	if (UEOK_Stats_QueryStats* Node = static_cast<UEOK_Stats_QueryStats*>(Data->ClientData))
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

void UEOK_Stats_QueryStats::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Stats_QueryStatsOptions Options;
			Options.ApiVersion = EOS_STATS_QUERYSTATS_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			Options.StartTime = Var_StartTime;
			Options.EndTime = Var_EndTime;
			Options.StatNamesCount = Var_StatNames.Num();
			char** TempStatNames = new char*[Var_StatNames.Num()];
			for (int i = 0; i < Var_StatNames.Num(); i++)
			{
				TempStatNames[i] = TCHAR_TO_ANSI(*Var_StatNames[i]);
			}
			const char** TempStatNamesConst = const_cast<const char**>(TempStatNames);
			Options.StatNames = TempStatNamesConst;
			EOS_Stats_QueryStats(EOSRef->StatsHandle, &Options, this, &UEOK_Stats_QueryStats::Internal_OnStatsQueryStatsComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_Stats_QueryStats::Activate: Failed to get EOS subsystem"));
	OnCallback.Broadcast(Var_LocalUserId, EEOK_Result::EOS_NotFound, Var_TargetUserId);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
