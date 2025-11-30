// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitStatsTypes.h"
#include "EOSKitStatsSubsystem.generated.h"

class UEOSKitSubsystem;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOSKitOnQueryStatsComplete, EEOSResult, Result, const FEOSKitProductUserId&, LocalUserId, const FEOSKitProductUserId&, TargetUserId);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOSKitOnIngestStatComplete, EEOSResult, Result, const FEOSKitProductUserId&, LocalUserId, const FEOSKitProductUserId&, TargetUserId);

/**
 * Stats Subsystem for EOSKit
 * Provides SDK function wrappers for stats operations
 */
UCLASS()
class EOSKITSTATS_API UEOSKitStatsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// Query Functions
	// ========================================

	/**
	 * Query for a list of stats for a specific player
	 * @param LocalUserId - Product User ID of the local user requesting the stats
	 * @param TargetUserId - Product User ID for the user whose stats are being retrieved
	 * @param StartTime - POSIX timestamp for start time, or -1 for undefined
	 * @param EndTime - POSIX timestamp for end time, or -1 for undefined
	 * @param StatNames - Array of stat names to query for (empty array queries all stats)
	 * @param Callback - Completion callback (required)
	 * @return true if the query was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Stats|Query")
	bool QueryStats(
		const FEOSKitProductUserId& LocalUserId,
		const FEOSKitProductUserId& TargetUserId,
		int64 StartTime,
		int64 EndTime,
		const TArray<FString>& StatNames,
		const FEOSKitOnQueryStatsComplete& Callback
	);

	/**
	 * Ingest a stat by the amount specified
	 * When the operation is complete, the stat will be uploaded to the backend to be processed
	 * @param LocalUserId - Product User ID of the local user requesting the ingest
	 * @param TargetUserId - Product User ID for the user whose stat is being ingested
	 * @param Stats - Array of stats to ingest
	 * @param Callback - Completion callback (required)
	 * @return true if the ingest was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Stats|Ingest")
	bool IngestStat(
		const FEOSKitProductUserId& LocalUserId,
		const FEOSKitProductUserId& TargetUserId,
		const TArray<FEOSKitStatsIngestData>& Stats,
		const FEOSKitOnIngestStatComplete& Callback
	);

	// ========================================
	// Stat Access Functions
	// ========================================

	/**
	 * Fetch the number of stats that are cached locally
	 * @param TargetUserId - Product User ID for the user whose stats are being counted
	 * @return Number of stats or 0 if there is an error
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Stats")
	int32 GetStatCount(const FEOSKitProductUserId& TargetUserId) const;

	/**
	 * Fetches a stat from a given index
	 * @param TargetUserId - Product User ID of the user who owns the stat
	 * @param StatIndex - Index of the stat to retrieve from the cache
	 * @param OutStat - The stat data if found
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Stats")
	EEOSResult CopyStatByIndex(
		const FEOSKitProductUserId& TargetUserId,
		int32 StatIndex,
		FEOSKitStat& OutStat
	) const;

	/**
	 * Fetches a stat from cached stats by name
	 * @param TargetUserId - Product User ID of the user who owns the stat
	 * @param StatName - Name of the stat to retrieve from the cache
	 * @param OutStat - The stat data if found
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Stats")
	EEOSResult CopyStatByName(
		const FEOSKitProductUserId& TargetUserId,
		const FString& StatName,
		FEOSKitStat& OutStat
	) const;

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HStats GetStatsHandle() const;

	// Callback handlers
	static void EOS_CALL OnQueryStatsCompleteCallback(const void* Data);
	static void EOS_CALL OnIngestStatCompleteCallback(const void* Data);

	// Delegates
	FEOSKitOnQueryStatsComplete OnQueryStatsCompleteDelegate;
	FEOSKitOnIngestStatComplete OnIngestStatCompleteDelegate;
};

