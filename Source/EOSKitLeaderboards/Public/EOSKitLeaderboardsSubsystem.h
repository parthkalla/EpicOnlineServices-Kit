// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitLeaderboardsTypes.h"
#include "EOSKitLeaderboardsSubsystem.generated.h"

class UEOSKitSubsystem;

DECLARE_DYNAMIC_DELEGATE_OneParam(FEOSKitOnQueryLeaderboardDefinitionsComplete, EEOSResult, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKitOnQueryLeaderboardRanksComplete, EEOSResult, Result, const FString&, LeaderboardId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOSKitOnQueryLeaderboardUserScoresComplete, EEOSResult, Result);

/**
 * Leaderboards Subsystem for EOSKit
 * Provides SDK function wrappers for leaderboard operations
 */
UCLASS()
class EOSKITLEADERBOARDS_API UEOSKitLeaderboardsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// Query Functions
	// ========================================

	/**
	 * Query for a list of existing leaderboard definitions including their attributes
	 * @param LocalUserId - Product User ID for user who is querying definitions
	 * @param StartTime - POSIX timestamp for start time, or -1 for undefined
	 * @param EndTime - POSIX timestamp for end time, or -1 for undefined
	 * @param Callback - Completion callback (required)
	 * @return true if the query was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|Query")
	bool QueryLeaderboardDefinitions(
		const FEOSKitProductUserId& LocalUserId,
		int64 StartTime,
		int64 EndTime,
		const FEOSKitOnQueryLeaderboardDefinitionsComplete& Callback
	);

	/**
	 * Retrieves top leaderboard records by rank in the leaderboard matching the given leaderboard ID
	 * @param LocalUserId - Product User ID for user who is querying ranks
	 * @param LeaderboardId - The ID of the leaderboard whose information you want to retrieve
	 * @param Callback - Completion callback (required)
	 * @return true if the query was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|Query")
	bool QueryLeaderboardRanks(
		const FEOSKitProductUserId& LocalUserId,
		const FString& LeaderboardId,
		const FEOSKitOnQueryLeaderboardRanksComplete& Callback
	);

	/**
	 * Query for a list of scores for a given list of users
	 * @param LocalUserId - Product User ID for user who is querying user scores
	 * @param UserIds - Array of Product User IDs indicating the users whose scores you want to retrieve
	 * @param StatInfo - The stats to be collected, along with the sorting method
	 * @param StartTime - POSIX timestamp, or -1 for undefined; results will only include scores made after this time
	 * @param EndTime - POSIX timestamp, or -1 for undefined; results will only include scores made before this time
	 * @param Callback - Completion callback (required)
	 * @return true if the query was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|Query")
	bool QueryLeaderboardUserScores(
		const FEOSKitProductUserId& LocalUserId,
		const TArray<FEOSKitProductUserId>& UserIds,
		const TArray<FEOSKitUserScoresQueryStatInfo>& StatInfo,
		int64 StartTime,
		int64 EndTime,
		const FEOSKitOnQueryLeaderboardUserScoresComplete& Callback
	);

	// ========================================
	// Definition Functions
	// ========================================

	/**
	 * Fetch the number of leaderboards definitions that are cached locally
	 * @return Number of leaderboards or 0 if there is an error
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Leaderboards|Definition")
	int32 GetLeaderboardDefinitionCount() const;

	/**
	 * Fetches a leaderboard definition from the cache using an index
	 * @param LeaderboardIndex - Index of the leaderboard definition to retrieve
	 * @param OutLeaderboardDefinition - The leaderboard definition if found
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|Definition")
	EEOSResult CopyLeaderboardDefinitionByIndex(int32 LeaderboardIndex, FEOSKitLeaderboardDefinition& OutLeaderboardDefinition) const;

	/**
	 * Fetches a leaderboard definition from the cache using a leaderboard ID
	 * @param LeaderboardId - The ID of the leaderboard whose definition you want to copy
	 * @param OutLeaderboardDefinition - The leaderboard definition if found
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|Definition")
	EEOSResult CopyLeaderboardDefinitionByLeaderboardId(const FString& LeaderboardId, FEOSKitLeaderboardDefinition& OutLeaderboardDefinition) const;

	// ========================================
	// Record Functions
	// ========================================

	/**
	 * Fetch the number of leaderboard records that are cached locally
	 * @return Number of leaderboard records or 0 if there is an error
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Leaderboards|Record")
	int32 GetLeaderboardRecordCount() const;

	/**
	 * Fetches a leaderboard record from a given index
	 * @param LeaderboardRecordIndex - Index of the leaderboard record to retrieve
	 * @param OutLeaderboardRecord - The leaderboard record if found
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|Record")
	EEOSResult CopyLeaderboardRecordByIndex(int32 LeaderboardRecordIndex, FEOSKitLeaderboardRecord& OutLeaderboardRecord) const;

	/**
	 * Fetches a leaderboard record from a given user ID
	 * @param UserId - Product User ID to look for
	 * @param OutLeaderboardRecord - The leaderboard record if found
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|Record")
	EEOSResult CopyLeaderboardRecordByUserId(const FEOSKitProductUserId& UserId, FEOSKitLeaderboardRecord& OutLeaderboardRecord) const;

	// ========================================
	// User Score Functions
	// ========================================

	/**
	 * Fetch the number of leaderboard user scores that are cached locally
	 * @param StatName - Name of stat used to rank leaderboard
	 * @return Number of leaderboard user scores or 0 if there is an error
	 */
	UFUNCTION(BlueprintPure, Category = "EOSKit|Leaderboards|UserScore")
	int32 GetLeaderboardUserScoreCount(const FString& StatName) const;

	/**
	 * Fetches leaderboard user score from a given index
	 * @param LeaderboardUserScoreIndex - Index of the sorted leaderboard user score to retrieve
	 * @param StatName - Name of the stat used to rank the leaderboard
	 * @param OutLeaderboardUserScore - The leaderboard user score if found
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|UserScore")
	EEOSResult CopyLeaderboardUserScoreByIndex(int32 LeaderboardUserScoreIndex, const FString& StatName, FEOSKitLeaderboardUserScore& OutLeaderboardUserScore) const;

	/**
	 * Fetches leaderboard user score from a given user ID
	 * @param UserId - The Product User ID to look for
	 * @param StatName - The name of the stat that is used to rank this leaderboard
	 * @param OutLeaderboardUserScore - The leaderboard user score if found
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboards|UserScore")
	EEOSResult CopyLeaderboardUserScoreByUserId(const FEOSKitProductUserId& UserId, const FString& StatName, FEOSKitLeaderboardUserScore& OutLeaderboardUserScore) const;

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HLeaderboards GetLeaderboardsHandle() const;

	// Callback handlers
	static void EOS_CALL OnQueryLeaderboardDefinitionsCompleteCallback(const void* Data);
	static void EOS_CALL OnQueryLeaderboardRanksCompleteCallback(const void* Data);
	static void EOS_CALL OnQueryLeaderboardUserScoresCompleteCallback(const void* Data);

	// Delegates
	FEOSKitOnQueryLeaderboardDefinitionsComplete OnQueryLeaderboardDefinitionsCompleteDelegate;
	FEOSKitOnQueryLeaderboardRanksComplete OnQueryLeaderboardRanksCompleteDelegate;
	FEOSKitOnQueryLeaderboardUserScoresComplete OnQueryLeaderboardUserScoresCompleteDelegate;
};

