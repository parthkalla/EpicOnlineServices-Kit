// Restored from original EOSKitLeaderboards module
#include "EOSKitLeaderboardsSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_leaderboards.h"
#include "eos_leaderboards_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"
#include "Async/Async.h"

void UEOSKitLeaderboardsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("EOSKitLeaderboardsSubsystem: Initialized"));
}

void UEOSKitLeaderboardsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitLeaderboardsSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HLeaderboards UEOSKitLeaderboardsSubsystem::GetLeaderboardsHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetLeaderboardsInterface(EOSKitSubsystem->GetPlatformHandle());
}

bool UEOSKitLeaderboardsSubsystem::QueryLeaderboardDefinitions(
	const FEOSKitProductUserId& LocalUserId,
	int64 StartTime,
	int64 EndTime,
	const FEOSKitOnQueryLeaderboardDefinitionsComplete& Callback)
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitLeaderboards: Failed to get Leaderboards Handle"));
		return false;
	}

	OnQueryLeaderboardDefinitionsCompleteDelegate = Callback;

	EOS_Leaderboards_QueryLeaderboardDefinitionsOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.StartTime = StartTime;
	Options.EndTime = EndTime;

	EOS_Leaderboards_QueryLeaderboardDefinitions(
		LeaderboardsHandle,
		&Options,
		this,
		&UEOSKitLeaderboardsSubsystem::OnQueryLeaderboardDefinitionsCompleteCallback
	);

	return true;
}

bool UEOSKitLeaderboardsSubsystem::QueryLeaderboardRanks(
	const FEOSKitProductUserId& LocalUserId,
	const FString& LeaderboardId,
	const FEOSKitOnQueryLeaderboardRanksComplete& Callback)
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitLeaderboards: Failed to get Leaderboards Handle"));
		return false;
	}

	OnQueryLeaderboardRanksCompleteDelegate = Callback;

	EOS_Leaderboards_QueryLeaderboardRanksOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDRANKS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.LeaderboardId = TCHAR_TO_UTF8(*LeaderboardId);

	EOS_Leaderboards_QueryLeaderboardRanks(
		LeaderboardsHandle,
		&Options,
		this,
		&UEOSKitLeaderboardsSubsystem::OnQueryLeaderboardRanksCompleteCallback
	);

	return true;
}

bool UEOSKitLeaderboardsSubsystem::QueryLeaderboardUserScores(
	const FEOSKitProductUserId& LocalUserId,
	const TArray<FEOSKitProductUserId>& UserIds,
	const TArray<FEOSKitUserScoresQueryStatInfo>& StatInfo,
	int64 StartTime,
	int64 EndTime,
	const FEOSKitOnQueryLeaderboardUserScoresComplete& Callback)
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitLeaderboards: Failed to get Leaderboards Handle"));
		return false;
	}

	if (UserIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitLeaderboards: UserIds array is empty"));
		return false;
	}

	if (StatInfo.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitLeaderboards: StatInfo array is empty"));
		return false;
	}

	OnQueryLeaderboardUserScoresCompleteDelegate = Callback;

	TArray<EOS_ProductUserId> EOSUserIds;
	EOSUserIds.Reserve(UserIds.Num());
	for (const FEOSKitProductUserId& UserId : UserIds)
	{
		EOSUserIds.Add(UserId.GetValueAsEosType());
	}

	TArray<EOS_Leaderboards_UserScoresQueryStatInfo> EOSStatInfo;
	EOSStatInfo.Reserve(StatInfo.Num());
	for (const FEOSKitUserScoresQueryStatInfo& Stat : StatInfo)
	{
		EOS_Leaderboards_UserScoresQueryStatInfo EOSStat = {};
		EOSStat.ApiVersion = EOS_LEADERBOARDS_USERSCORESQUERYSTATINFO_API_LATEST;
		EOSStat.StatName = TCHAR_TO_UTF8(*Stat.StatName);
		EOSStat.Aggregation = static_cast<EOS_ELeaderboardAggregation>(Stat.Aggregation);
		EOSStatInfo.Add(EOSStat);
	}

	EOS_Leaderboards_QueryLeaderboardUserScoresOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDUSERSCORES_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.UserIds = EOSUserIds.GetData();
	Options.UserIdsCount = EOSUserIds.Num();
	Options.StatInfo = EOSStatInfo.GetData();
	Options.StatInfoCount = EOSStatInfo.Num();
	Options.StartTime = StartTime;
	Options.EndTime = EndTime;

	EOS_Leaderboards_QueryLeaderboardUserScores(
		LeaderboardsHandle,
		&Options,
		this,
		&UEOSKitLeaderboardsSubsystem::OnQueryLeaderboardUserScoresCompleteCallback
	);

	return true;
}

int32 UEOSKitLeaderboardsSubsystem::GetLeaderboardDefinitionCount() const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return 0;
	}

	EOS_Leaderboards_GetLeaderboardDefinitionCountOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDDEFINITIONCOUNT_API_LATEST;

	return static_cast<int32>(EOS_Leaderboards_GetLeaderboardDefinitionCount(LeaderboardsHandle, &Options));
}

EEOSResult UEOSKitLeaderboardsSubsystem::CopyLeaderboardDefinitionByIndex(
	int32 LeaderboardIndex,
	FEOSKitLeaderboardDefinition& OutLeaderboardDefinition) const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYINDEX_API_LATEST;
	Options.LeaderboardIndex = static_cast<uint32_t>(LeaderboardIndex);

	EOS_Leaderboards_Definition* LeaderboardDefinition = nullptr;
	EOS_EResult Result = EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(
		LeaderboardsHandle,
		&Options,
		&LeaderboardDefinition
	);

	if (Result == EOS_EResult::EOS_Success && LeaderboardDefinition)
	{
		OutLeaderboardDefinition.LeaderboardId = UTF8_TO_TCHAR(LeaderboardDefinition->LeaderboardId);
		OutLeaderboardDefinition.StatName = UTF8_TO_TCHAR(LeaderboardDefinition->StatName);
		OutLeaderboardDefinition.Aggregation = static_cast<EEOSKitLeaderboardAggregation>(LeaderboardDefinition->Aggregation);
		OutLeaderboardDefinition.StartTime = LeaderboardDefinition->StartTime;
		OutLeaderboardDefinition.EndTime = LeaderboardDefinition->EndTime;

		EOS_Leaderboards_Definition_Release(LeaderboardDefinition);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitLeaderboardsSubsystem::CopyLeaderboardDefinitionByLeaderboardId(
	const FString& LeaderboardId,
	FEOSKitLeaderboardDefinition& OutLeaderboardDefinition) const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYLEADERBOARDID_API_LATEST;
	Options.LeaderboardId = TCHAR_TO_UTF8(*LeaderboardId);

	EOS_Leaderboards_Definition* LeaderboardDefinition = nullptr;
	EOS_EResult Result = EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(
		LeaderboardsHandle,
		&Options,
		&LeaderboardDefinition
	);

	if (Result == EOS_EResult::EOS_Success && LeaderboardDefinition)
	{
		OutLeaderboardDefinition.LeaderboardId = UTF8_TO_TCHAR(LeaderboardDefinition->LeaderboardId);
		OutLeaderboardDefinition.StatName = UTF8_TO_TCHAR(LeaderboardDefinition->StatName);
		OutLeaderboardDefinition.Aggregation = static_cast<EEOSKitLeaderboardAggregation>(LeaderboardDefinition->Aggregation);
		OutLeaderboardDefinition.StartTime = LeaderboardDefinition->StartTime;
		OutLeaderboardDefinition.EndTime = LeaderboardDefinition->EndTime;

		EOS_Leaderboards_Definition_Release(LeaderboardDefinition);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

int32 UEOSKitLeaderboardsSubsystem::GetLeaderboardRecordCount() const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return 0;
	}

	EOS_Leaderboards_GetLeaderboardRecordCountOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDRECORDCOUNT_API_LATEST;

	return static_cast<int32>(EOS_Leaderboards_GetLeaderboardRecordCount(LeaderboardsHandle, &Options));
}

EEOSResult UEOSKitLeaderboardsSubsystem::CopyLeaderboardRecordByIndex(
	int32 LeaderboardRecordIndex,
	FEOSKitLeaderboardRecord& OutLeaderboardRecord) const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYINDEX_API_LATEST;
	Options.LeaderboardRecordIndex = static_cast<uint32_t>(LeaderboardRecordIndex);

	EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord = nullptr;
	EOS_EResult Result = EOS_Leaderboards_CopyLeaderboardRecordByIndex(
		LeaderboardsHandle,
		&Options,
		&LeaderboardRecord
	);

	if (Result == EOS_EResult::EOS_Success && LeaderboardRecord)
	{
		OutLeaderboardRecord.UserId = FEOSKitProductUserId(LeaderboardRecord->UserId);
		OutLeaderboardRecord.Score = LeaderboardRecord->Score;
		OutLeaderboardRecord.Rank = LeaderboardRecord->Rank;

		EOS_Leaderboards_LeaderboardRecord_Release(LeaderboardRecord);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitLeaderboardsSubsystem::CopyLeaderboardRecordByUserId(
	const FEOSKitProductUserId& UserId,
	FEOSKitLeaderboardRecord& OutLeaderboardRecord) const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYUSERID_API_LATEST;
	Options.UserId = UserId.GetValueAsEosType();

	EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord = nullptr;
	EOS_EResult Result = EOS_Leaderboards_CopyLeaderboardRecordByUserId(
		LeaderboardsHandle,
		&Options,
		&LeaderboardRecord
	);

	if (Result == EOS_EResult::EOS_Success && LeaderboardRecord)
	{
		OutLeaderboardRecord.UserId = FEOSKitProductUserId(LeaderboardRecord->UserId);
		OutLeaderboardRecord.Score = LeaderboardRecord->Score;
		OutLeaderboardRecord.Rank = LeaderboardRecord->Rank;

		EOS_Leaderboards_LeaderboardRecord_Release(LeaderboardRecord);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

int32 UEOSKitLeaderboardsSubsystem::GetLeaderboardUserScoreCount(const FString& StatName) const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return 0;
	}

	EOS_Leaderboards_GetLeaderboardUserScoreCountOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDUSERSCORECOUNT_API_LATEST;
	Options.StatName = TCHAR_TO_UTF8(*StatName);

	return static_cast<int32>(EOS_Leaderboards_GetLeaderboardUserScoreCount(LeaderboardsHandle, &Options));
}

EEOSResult UEOSKitLeaderboardsSubsystem::CopyLeaderboardUserScoreByIndex(
	int32 LeaderboardUserScoreIndex,
	const FString& StatName,
	FEOSKitLeaderboardUserScore& OutLeaderboardUserScore) const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYINDEX_API_LATEST;
	Options.LeaderboardUserScoreIndex = static_cast<uint32_t>(LeaderboardUserScoreIndex);
	Options.StatName = TCHAR_TO_UTF8(*StatName);

	EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScore = nullptr;
	EOS_EResult Result = EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(
		LeaderboardsHandle,
		&Options,
		&LeaderboardUserScore
	);

	if (Result == EOS_EResult::EOS_Success && LeaderboardUserScore)
	{
		OutLeaderboardUserScore.UserId = FEOSKitProductUserId(LeaderboardUserScore->UserId);
		OutLeaderboardUserScore.Score = LeaderboardUserScore->Score;

		EOS_Leaderboards_LeaderboardUserScore_Release(LeaderboardUserScore);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

EEOSResult UEOSKitLeaderboardsSubsystem::CopyLeaderboardUserScoreByUserId(
	const FEOSKitProductUserId& UserId,
	const FString& StatName,
	FEOSKitLeaderboardUserScore& OutLeaderboardUserScore) const
{
	EOS_HLeaderboards LeaderboardsHandle = GetLeaderboardsHandle();
	if (!LeaderboardsHandle)
	{
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions Options = {};
	Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYUSERID_API_LATEST;
	Options.UserId = UserId.GetValueAsEosType();
	Options.StatName = TCHAR_TO_UTF8(*StatName);

	EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScore = nullptr;
	EOS_EResult Result = EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(
		LeaderboardsHandle,
		&Options,
		&LeaderboardUserScore
	);

	if (Result == EOS_EResult::EOS_Success && LeaderboardUserScore)
	{
		OutLeaderboardUserScore.UserId = FEOSKitProductUserId(LeaderboardUserScore->UserId);
		OutLeaderboardUserScore.Score = LeaderboardUserScore->Score;

		EOS_Leaderboards_LeaderboardUserScore_Release(LeaderboardUserScore);
	}

	return ConvertEOSResultToEEOSResult(Result);
}

void EOS_CALL UEOSKitLeaderboardsSubsystem::OnQueryLeaderboardDefinitionsCompleteCallback(const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitLeaderboardsSubsystem* Self = static_cast<UEOSKitLeaderboardsSubsystem*>(Data->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(Data->ResultCode);

	if (Self->OnQueryLeaderboardDefinitionsCompleteDelegate.IsBound())
	{
		Self->OnQueryLeaderboardDefinitionsCompleteDelegate.Execute(Result);
	}
}

void EOS_CALL UEOSKitLeaderboardsSubsystem::OnQueryLeaderboardRanksCompleteCallback(const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitLeaderboardsSubsystem* Self = static_cast<UEOSKitLeaderboardsSubsystem*>(Data->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(Data->ResultCode);

	if (Self->OnQueryLeaderboardRanksCompleteDelegate.IsBound())
	{
		FString LeaderboardId = Data->LeaderboardId ? UTF8_TO_TCHAR(Data->LeaderboardId) : TEXT("");
		Self->OnQueryLeaderboardRanksCompleteDelegate.Execute(Result, LeaderboardId);
	}
}

void EOS_CALL UEOSKitLeaderboardsSubsystem::OnQueryLeaderboardUserScoresCompleteCallback(const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitLeaderboardsSubsystem* Self = static_cast<UEOSKitLeaderboardsSubsystem*>(Data->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(Data->ResultCode);

	if (Self->OnQueryLeaderboardUserScoresCompleteDelegate.IsBound())
	{
		Self->OnQueryLeaderboardUserScoresCompleteDelegate.Execute(Result);
	}
}


