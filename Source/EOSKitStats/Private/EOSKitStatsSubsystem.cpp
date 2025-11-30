// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitStatsSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_stats.h"
#include "eos_stats_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"
#include "Async/Async.h"

void UEOSKitStatsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("EOSKitStatsSubsystem: Initialized"));
}

void UEOSKitStatsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitStatsSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HStats UEOSKitStatsSubsystem::GetStatsHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetStatsInterface(EOSKitSubsystem->GetPlatformHandle());
}

bool UEOSKitStatsSubsystem::QueryStats(
	const FEOSKitProductUserId& LocalUserId,
	const FEOSKitProductUserId& TargetUserId,
	int64 StartTime,
	int64 EndTime,
	const TArray<FString>& StatNames,
	const FEOSKitOnQueryStatsComplete& Callback)
{
	EOS_HStats StatsHandle = GetStatsHandle();
	if (!StatsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitStats: Failed to get Stats Handle"));
		return false;
	}

	OnQueryStatsCompleteDelegate = Callback;

	EOS_Stats_QueryStatsOptions Options = {};
	Options.ApiVersion = EOS_STATS_QUERYSTATS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.StartTime = StartTime;
	Options.EndTime = EndTime;

	// Convert StatNames array if provided
	TArray<const char*> StatNamePtrs;
	TArray<FString> StatNameStrings;
	if (StatNames.Num() > 0)
	{
		if (StatNames.Num() > EOS_STATS_MAX_QUERY_STATS)
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSKitStats: StatNames array exceeds maximum (%d), truncating to %d"), StatNames.Num(), EOS_STATS_MAX_QUERY_STATS);
		}

		int32 MaxStats = FMath::Min(StatNames.Num(), static_cast<int32>(EOS_STATS_MAX_QUERY_STATS));
		StatNameStrings.Reserve(MaxStats);
		StatNamePtrs.Reserve(MaxStats);

		for (int32 i = 0; i < MaxStats; i++)
		{
			StatNameStrings.Add(StatNames[i]);
			StatNamePtrs.Add(TCHAR_TO_UTF8(*StatNameStrings[i]));
		}

		Options.StatNames = StatNamePtrs.GetData();
		Options.StatNamesCount = static_cast<uint32_t>(MaxStats);
	}
	else
	{
		Options.StatNames = nullptr;
		Options.StatNamesCount = 0;
	}

	EOS_Stats_QueryStats(
		StatsHandle,
		&Options,
		this,
		&UEOSKitStatsSubsystem::OnQueryStatsCompleteCallback
	);

	return true;
}

bool UEOSKitStatsSubsystem::IngestStat(
	const FEOSKitProductUserId& LocalUserId,
	const FEOSKitProductUserId& TargetUserId,
	const TArray<FEOSKitStatsIngestData>& Stats,
	const FEOSKitOnIngestStatComplete& Callback)
{
	EOS_HStats StatsHandle = GetStatsHandle();
	if (!StatsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitStats: Failed to get Stats Handle"));
		return false;
	}

	if (Stats.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitStats: Stats array is empty"));
		return false;
	}

	if (Stats.Num() > EOS_STATS_MAX_INGEST_STATS)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitStats: Stats array exceeds maximum (%d), maximum is %d"), Stats.Num(), EOS_STATS_MAX_INGEST_STATS);
		return false;
	}

	OnIngestStatCompleteDelegate = Callback;

	// Convert Stats array to EOS format
	TArray<EOS_Stats_IngestData> EOSStats;
	EOSStats.Reserve(Stats.Num());
	for (const FEOSKitStatsIngestData& Stat : Stats)
	{
		EOS_Stats_IngestData EOSStat = {};
		EOSStat.ApiVersion = EOS_STATS_INGESTDATA_API_LATEST;
		EOSStat.StatName = TCHAR_TO_UTF8(*Stat.StatName);
		EOSStat.IngestAmount = Stat.IngestAmount;
		EOSStats.Add(EOSStat);
	}

	EOS_Stats_IngestStatOptions Options = {};
	Options.ApiVersion = EOS_STATS_INGESTSTAT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.Stats = EOSStats.GetData();
	Options.StatsCount = static_cast<uint32_t>(EOSStats.Num());

	EOS_Stats_IngestStat(
		StatsHandle,
		&Options,
		this,
		&UEOSKitStatsSubsystem::OnIngestStatCompleteCallback
	);

	return true;
}

int32 UEOSKitStatsSubsystem::GetStatCount(const FEOSKitProductUserId& TargetUserId) const
{
	EOS_HStats StatsHandle = GetStatsHandle();
	if (!StatsHandle)
	{
		return 0;
	}

	EOS_Stats_GetStatCountOptions Options = {};
	Options.ApiVersion = EOS_STATS_GETSTATSCOUNT_API_LATEST;
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	return static_cast<int32>(EOS_Stats_GetStatsCount(StatsHandle, &Options));
}

EEOSResult UEOSKitStatsSubsystem::CopyStatByIndex(
	const FEOSKitProductUserId& TargetUserId,
	int32 StatIndex,
	FEOSKitStat& OutStat) const
{
	EOS_HStats StatsHandle = GetStatsHandle();
	if (!StatsHandle)
	{
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Stats_CopyStatByIndexOptions Options = {};
	Options.ApiVersion = EOS_STATS_COPYSTATBYINDEX_API_LATEST;
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.StatIndex = static_cast<uint32_t>(StatIndex);

	EOS_Stats_Stat* Stat = nullptr;
	EOS_EResult Result = EOS_Stats_CopyStatByIndex(
		StatsHandle,
		&Options,
		&Stat
	);

	if (Result == EOS_EResult::EOS_Success && Stat)
	{
		OutStat.Name = UTF8_TO_TCHAR(Stat->Name);
		OutStat.StartTime = Stat->StartTime;
		OutStat.EndTime = Stat->EndTime;
		OutStat.Value = Stat->Value;

		EOS_Stats_Stat_Release(Stat);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitStatsSubsystem::CopyStatByName(
	const FEOSKitProductUserId& TargetUserId,
	const FString& StatName,
	FEOSKitStat& OutStat) const
{
	EOS_HStats StatsHandle = GetStatsHandle();
	if (!StatsHandle)
	{
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Stats_CopyStatByNameOptions Options = {};
	Options.ApiVersion = EOS_STATS_COPYSTATBYNAME_API_LATEST;
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.Name = TCHAR_TO_UTF8(*StatName);

	EOS_Stats_Stat* Stat = nullptr;
	EOS_EResult Result = EOS_Stats_CopyStatByName(
		StatsHandle,
		&Options,
		&Stat
	);

	if (Result == EOS_EResult::EOS_Success && Stat)
	{
		OutStat.Name = UTF8_TO_TCHAR(Stat->Name);
		OutStat.StartTime = Stat->StartTime;
		OutStat.EndTime = Stat->EndTime;
		OutStat.Value = Stat->Value;

		EOS_Stats_Stat_Release(Stat);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

void EOS_CALL UEOSKitStatsSubsystem::OnQueryStatsCompleteCallback(const void* Data)
{
	const EOS_Stats_OnQueryStatsCompleteCallbackInfo* CallbackInfo =
		static_cast<const EOS_Stats_OnQueryStatsCompleteCallbackInfo*>(Data);

	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitStatsSubsystem* Subsystem = static_cast<UEOSKitStatsSubsystem*>(CallbackInfo->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(CallbackInfo->ResultCode);
	FEOSKitProductUserId LocalUserId(CallbackInfo->LocalUserId);
	FEOSKitProductUserId TargetUserId(CallbackInfo->TargetUserId);

	AsyncTask(ENamedThreads::GameThread, [Subsystem, Result, LocalUserId, TargetUserId]()
	{
		if (Subsystem)
		{
			Subsystem->OnQueryStatsCompleteDelegate.ExecuteIfBound(Result, LocalUserId, TargetUserId);
		}
	});
}

void EOS_CALL UEOSKitStatsSubsystem::OnIngestStatCompleteCallback(const void* Data)
{
	const EOS_Stats_IngestStatCompleteCallbackInfo* CallbackInfo =
		static_cast<const EOS_Stats_IngestStatCompleteCallbackInfo*>(Data);

	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSKitStatsSubsystem* Subsystem = static_cast<UEOSKitStatsSubsystem*>(CallbackInfo->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(CallbackInfo->ResultCode);
	FEOSKitProductUserId LocalUserId(CallbackInfo->LocalUserId);
	FEOSKitProductUserId TargetUserId(CallbackInfo->TargetUserId);

	AsyncTask(ENamedThreads::GameThread, [Subsystem, Result, LocalUserId, TargetUserId]()
	{
		if (Subsystem)
		{
			Subsystem->OnIngestStatCompleteDelegate.ExecuteIfBound(Result, LocalUserId, TargetUserId);
		}
	});
}

