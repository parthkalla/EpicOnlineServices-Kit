// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_StatsSubsystem.h"

#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

TEnumAsByte<EEOK_Result> UEOK_StatsSubsystem::EOK_Stats_CopyStatByIndex(FEOK_ProductUserId TargetUserId,
                                                                        int32 StatIndex, FEOK_Stats_Stat& OutStat)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Stats_CopyStatByIndexOptions Options;
			Options.ApiVersion = EOS_STATS_COPYSTATBYINDEX_API_LATEST;
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			Options.StatIndex = StatIndex;
			EOS_Stats_Stat *Stat = nullptr;
			auto Result = EOS_Stats_CopyStatByIndex(EOSRef->StatsHandle, &Options, &Stat);
			if (Result == EOS_EResult::EOS_Success)
			{
				OutStat = *Stat;
				EOS_Stats_Stat_Release(Stat);
			}
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_StatsSubsystem::EOK_Stats_CopyStatByIndex: Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_StatsSubsystem::EOK_Stats_CopyStatByName(FEOK_ProductUserId TargetUserId,
	const FString& Name, FEOK_Stats_Stat& OutStat)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Stats_CopyStatByNameOptions Options;
			Options.ApiVersion = EOS_STATS_COPYSTATBYNAME_API_LATEST;
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			Options.Name = TCHAR_TO_ANSI(*Name);
			EOS_Stats_Stat *Stat = nullptr;
			auto Result = EOS_Stats_CopyStatByName(EOSRef->StatsHandle, &Options, &Stat);
			if (Result == EOS_EResult::EOS_Success)
			{
				OutStat = *Stat;
				EOS_Stats_Stat_Release(Stat);
			}
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_StatsSubsystem::EOK_Stats_CopyStatByName: Failed to get EOS subsystem"));
	return EEOK_Result::EOS_NotFound;
}

int32 UEOK_StatsSubsystem::EOK_Stats_GetStatsCount(FEOK_ProductUserId TargetUserId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Stats_GetStatCountOptions Options;
			Options.ApiVersion = EOS_STATS_GETSTATSCOUNT_API_LATEST;
			Options.TargetUserId = TargetUserId.GetValueAsEosType();
			return EOS_Stats_GetStatsCount(EOSRef->StatsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_StatsSubsystem::EOK_Stats_GetStatsCount: Failed to get EOS subsystem"));
	return 0;
}
