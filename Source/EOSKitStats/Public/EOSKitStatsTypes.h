// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitStatsTypes.generated.h"

/**
 * Stats ingest data structure
 */
USTRUCT(BlueprintType)
struct FEOSKitStatsIngestData
{
	GENERATED_BODY()

	/** The name of the stat to ingest */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Stats")
	FString StatName = TEXT("");

	/** The amount to ingest the stat */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Stats")
	int32 IngestAmount = 0;

	FEOSKitStatsIngestData()
		: StatName(TEXT(""))
		, IngestAmount(0)
	{
	}
};

/**
 * Stats structure
 */
USTRUCT(BlueprintType)
struct FEOSKitStat
{
	GENERATED_BODY()

	/** Name of the stat */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Stats")
	FString Name = TEXT("");

	/** If not -1 then this is the POSIX timestamp for start time */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Stats")
	int64 StartTime = -1;

	/** If not -1 then this is the POSIX timestamp for end time */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Stats")
	int64 EndTime = -1;

	/** Current value for the stat */
	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Stats")
	int32 Value = 0;

	FEOSKitStat()
		: Name(TEXT(""))
		, StartTime(-1)
		, EndTime(-1)
		, Value(0)
	{
	}
};

