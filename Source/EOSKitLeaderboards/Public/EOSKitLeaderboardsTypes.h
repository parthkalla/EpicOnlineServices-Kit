// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitLeaderboardsTypes.generated.h"

/**
 * Leaderboard aggregation type enumeration
 */
UENUM(BlueprintType)
enum class EEOSKitLeaderboardAggregation : uint8
{
	Min = 0		UMETA(DisplayName = "Minimum"),
	Max = 1		UMETA(DisplayName = "Maximum"),
	Sum = 2		UMETA(DisplayName = "Sum"),
	Latest = 3	UMETA(DisplayName = "Latest")
};

/**
 * Leaderboard definition structure
 */
USTRUCT(BlueprintType)
struct FEOSKitLeaderboardDefinition
{
	GENERATED_BODY()

	/** Unique ID to identify leaderboard */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	FString LeaderboardId = TEXT("");

	/** Name of stat used to rank leaderboard */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	FString StatName = TEXT("");

	/** Aggregation used to sort leaderboard */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	EEOSKitLeaderboardAggregation Aggregation = EEOSKitLeaderboardAggregation::Min;

	/** The POSIX timestamp for the start time, or EOS_LEADERBOARDS_TIME_UNDEFINED */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	int64 StartTime = -1;

	/** The POSIX timestamp for the end time, or EOS_LEADERBOARDS_TIME_UNDEFINED */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	int64 EndTime = -1;

	FEOSKitLeaderboardDefinition()
		: LeaderboardId(TEXT(""))
		, StatName(TEXT(""))
		, Aggregation(EEOSKitLeaderboardAggregation::Min)
		, StartTime(-1)
		, EndTime(-1)
	{
	}
};

/**
 * Leaderboard record structure
 */
USTRUCT(BlueprintType)
struct FEOSKitLeaderboardRecord
{
	GENERATED_BODY()

	/** The Product User ID associated with this record */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	FEOSKitProductUserId UserId;

	/** Sorted position on leaderboard */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	int32 Rank = 0;

	/** Leaderboard score */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	int32 Score = 0;

	/** The latest display name seen for the user since they last time logged in */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	FString UserDisplayName = TEXT("");

	FEOSKitLeaderboardRecord()
		: UserId()
		, Rank(0)
		, Score(0)
		, UserDisplayName(TEXT(""))
	{
	}
};

/**
 * Leaderboard user score structure
 */
USTRUCT(BlueprintType)
struct FEOSKitLeaderboardUserScore
{
	GENERATED_BODY()

	/** The Product User ID of the user who got this score */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	FEOSKitProductUserId UserId;

	/** Leaderboard score */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Leaderboards")
	int32 Score = 0;

	FEOSKitLeaderboardUserScore()
		: UserId()
		, Score(0)
	{
	}
};

/**
 * User scores query stat info structure
 */
USTRUCT(BlueprintType)
struct FEOSKitUserScoresQueryStatInfo
{
	GENERATED_BODY()

	/** The name of the stat to query */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Leaderboards")
	FString StatName = TEXT("");

	/** Aggregation used to sort the cached user scores */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Leaderboards")
	EEOSKitLeaderboardAggregation Aggregation = EEOSKitLeaderboardAggregation::Min;

	FEOSKitUserScoresQueryStatInfo()
		: StatName(TEXT(""))
		, Aggregation(EEOSKitLeaderboardAggregation::Min)
	{
	}
};

