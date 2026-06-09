// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_LeaderboardsSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

TEnumAsByte<EEOK_Result> UEOK_LeaderboardsSubsystem::EOK_Leaderboards_CopyLeaderboardDefinitionByIndex(
	int32 LeaderboardIndex, FEOK_Leaderboards_Definition& OutLeaderboardDefinition)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYINDEX_API_LATEST;
			Options.LeaderboardIndex = LeaderboardIndex;
			EOS_Leaderboards_Definition* LeaderboardDefinition = nullptr;
			EEOK_Result Result = static_cast<EEOK_Result>(EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(EOSRef->LeaderboardsHandle, &Options, &LeaderboardDefinition));
			if (Result == EEOK_Result::EOS_Success)
			{
				OutLeaderboardDefinition = *LeaderboardDefinition;
			}
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to copy leaderboard definition by index either OnlineSubsystem is not valid or EOSRef is not valid."));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_LeaderboardsSubsystem::EOK_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(
	FString LeaderboardId, FEOK_Leaderboards_Definition& OutLeaderboardDefinition)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYLEADERBOARDID_API_LATEST;
			Options.LeaderboardId = TCHAR_TO_ANSI(*LeaderboardId);
			EOS_Leaderboards_Definition* LeaderboardDefinition = nullptr;
			EEOK_Result Result = static_cast<EEOK_Result>(EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(EOSRef->LeaderboardsHandle, &Options, &LeaderboardDefinition));
			if (Result == EEOK_Result::EOS_Success)
			{
				OutLeaderboardDefinition = *LeaderboardDefinition;
			}
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to copy leaderboard definition by leaderboard id either OnlineSubsystem is not valid or EOSRef is not valid."));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_LeaderboardsSubsystem::EOK_Leaderboards_CopyLeaderboardRecordByIndex(
	int32 LeaderboardRecordIndex, FEOK_Leaderboards_LeaderboardRecord& OutLeaderboardRecord)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYINDEX_API_LATEST;
			Options.LeaderboardRecordIndex = LeaderboardRecordIndex;
			EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord = nullptr;
			EEOK_Result Result = static_cast<EEOK_Result>(EOS_Leaderboards_CopyLeaderboardRecordByIndex(EOSRef->LeaderboardsHandle, &Options, &LeaderboardRecord));
			if (Result == EEOK_Result::EOS_Success)
			{
				OutLeaderboardRecord = *LeaderboardRecord;
			}
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to copy leaderboard record by index either OnlineSubsystem is not valid or EOSRef is not valid."));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_LeaderboardsSubsystem::EOK_Leaderboards_CopyLeaderboardRecordByUserId(
	FEOK_ProductUserId UserId, FEOK_Leaderboards_LeaderboardRecord& OutLeaderboardRecord)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYUSERID_API_LATEST;
			Options.UserId = UserId.GetValueAsEosType();
			EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord = nullptr;
			EEOK_Result Result = static_cast<EEOK_Result>(EOS_Leaderboards_CopyLeaderboardRecordByUserId(EOSRef->LeaderboardsHandle, &Options, &LeaderboardRecord));
			if (Result == EEOK_Result::EOS_Success)
			{
				OutLeaderboardRecord = *LeaderboardRecord;
			}
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to copy leaderboard record by user id either OnlineSubsystem is not valid or EOSRef is not valid."));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_LeaderboardsSubsystem::EOK_Leaderboards_CopyLeaderboardUserScoreByIndex(
	int32 LeaderboardUserScoreIndex, FString StatName, FEOK_Leaderboards_LeaderboardUserScore& OutLeaderboardUserScore)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYINDEX_API_LATEST;
			Options.LeaderboardUserScoreIndex = LeaderboardUserScoreIndex;
			Options.StatName = TCHAR_TO_ANSI(*StatName);
			EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScore = nullptr;
			EEOK_Result Result = static_cast<EEOK_Result>(EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(EOSRef->LeaderboardsHandle, &Options, &LeaderboardUserScore));
			if (Result == EEOK_Result::EOS_Success)
			{
				OutLeaderboardUserScore = *LeaderboardUserScore;
			}
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to copy leaderboard user score by index either OnlineSubsystem is not valid or EOSRef is not valid."));
	return EEOK_Result::EOS_NotFound;
}

TEnumAsByte<EEOK_Result> UEOK_LeaderboardsSubsystem::EOK_Leaderboards_CopyLeaderboardUserScoreByUserId(
	FEOK_ProductUserId UserId, FString StatName, FEOK_Leaderboards_LeaderboardUserScore& OutLeaderboardUserScore)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYUSERID_API_LATEST;
			Options.UserId = UserId.GetValueAsEosType();
			Options.StatName = TCHAR_TO_ANSI(*StatName);
			EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScore = nullptr;
			EEOK_Result Result = static_cast<EEOK_Result>(EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(EOSRef->LeaderboardsHandle, &Options, &LeaderboardUserScore));
			if (Result == EEOK_Result::EOS_Success)
			{
				OutLeaderboardUserScore = *LeaderboardUserScore;
			}
			return Result;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to copy leaderboard user score by user id either OnlineSubsystem is not valid or EOSRef is not valid."));
	return EEOK_Result::EOS_NotFound;
}

void UEOK_LeaderboardsSubsystem::EOK_Leaderboards_LeaderboardDefinition_Release(
	FEOK_Leaderboards_Definition& LeaderboardDefinition)
{
	EOS_Leaderboards_Definition* LeaderboardDefinitionPtr = &LeaderboardDefinition.Ref;
	EOS_Leaderboards_Definition_Release(LeaderboardDefinitionPtr);
}

int32 UEOK_LeaderboardsSubsystem::EOK_Leaderboards_GetLeaderboardDefinitionCount()
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_GetLeaderboardDefinitionCountOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDDEFINITIONCOUNT_API_LATEST;
			return EOS_Leaderboards_GetLeaderboardDefinitionCount(EOSRef->LeaderboardsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get leaderboard definition count either OnlineSubsystem is not valid or EOSRef is not valid."));
	return 0;
}

int32 UEOK_LeaderboardsSubsystem::EOK_Leaderboards_GetLeaderboardRecordCount()
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_GetLeaderboardRecordCountOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDRECORDCOUNT_API_LATEST;
			return EOS_Leaderboards_GetLeaderboardRecordCount(EOSRef->LeaderboardsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get leaderboard record count either OnlineSubsystem is not valid or EOSRef is not valid."));
	return 0;
}

int32 UEOK_LeaderboardsSubsystem::EOK_Leaderboards_GetLeaderboardUserScoreCount()
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Leaderboards_GetLeaderboardUserScoreCountOptions Options = {};
			Options.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDUSERSCORECOUNT_API_LATEST;
			return EOS_Leaderboards_GetLeaderboardUserScoreCount(EOSRef->LeaderboardsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get leaderboard user score count either OnlineSubsystem is not valid or EOSRef is not valid."));
	return 0;
}

void UEOK_LeaderboardsSubsystem::EOK_Leaderboards_LeaderboardRecord_Release(
	FEOK_Leaderboards_LeaderboardRecord& LeaderboardRecord)
{
	EOS_Leaderboards_LeaderboardRecord* LeaderboardRecordPtr = &LeaderboardRecord.Ref;
	EOS_Leaderboards_LeaderboardRecord_Release(LeaderboardRecordPtr);
}

void UEOK_LeaderboardsSubsystem::EOK_Leaderboards_LeaderboardUserScore_Release(
	FEOK_Leaderboards_LeaderboardUserScore& LeaderboardUserScore)
{
	EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScorePtr = &LeaderboardUserScore.Ref;
	EOS_Leaderboards_LeaderboardUserScore_Release(LeaderboardUserScorePtr);
}
