// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_leaderboards.h"
#include "eos_leaderboards_types.h"
THIRD_PARTY_INCLUDES_END
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_LeaderboardsSubsystem.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Leaderboards Interface", meta=(DisplayName="Leaderboards Interface"))
class OnlineSubsystemEOK_API UEOK_LeaderboardsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	//Fetches a leaderboard definition from the cache using an index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardDefinitionByIndex")
	TEnumAsByte<EEOK_Result> EOK_Leaderboards_CopyLeaderboardDefinitionByIndex(int32 LeaderboardIndex, FEOK_Leaderboards_Definition& OutLeaderboardDefinition);

	//Fetches a leaderboard definition from the cache using a leaderboard ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId")
	TEnumAsByte<EEOK_Result> EOK_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(FString LeaderboardId, FEOK_Leaderboards_Definition& OutLeaderboardDefinition);

	//Fetches a leaderboard record from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardRecordByIndex")
	TEnumAsByte<EEOK_Result> EOK_Leaderboards_CopyLeaderboardRecordByIndex(int32 LeaderboardRecordIndex, FEOK_Leaderboards_LeaderboardRecord& OutLeaderboardRecord);

	//Fetches a leaderboard record from a given user ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardRecordByUserId")
	TEnumAsByte<EEOK_Result> EOK_Leaderboards_CopyLeaderboardRecordByUserId(FEOK_ProductUserId UserId, FEOK_Leaderboards_LeaderboardRecord& OutLeaderboardRecord);

	//Fetches leaderboard user score from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardUserScoreByIndex")
	TEnumAsByte<EEOK_Result> EOK_Leaderboards_CopyLeaderboardUserScoreByIndex(int32 LeaderboardUserScoreIndex, FString StatName, FEOK_Leaderboards_LeaderboardUserScore& OutLeaderboardUserScore);

	//Fetches leaderboard user score from a given user ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_CopyLeaderboardUserScoreByUserId")
	TEnumAsByte<EEOK_Result> EOK_Leaderboards_CopyLeaderboardUserScoreByUserId(FEOK_ProductUserId UserId, FString StatName, FEOK_Leaderboards_LeaderboardUserScore& OutLeaderboardUserScore);

	//Release the memory associated with a leaderboard definition. This must be called on data retrieved from EOS_Leaderboards_CopyLeaderboardDefinitionByIndex or EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_Definition_Release")
	void EOK_Leaderboards_LeaderboardDefinition_Release(FEOK_Leaderboards_Definition& LeaderboardDefinition);

	//Fetch the number of leaderboards definitions that are cached locally.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_GetLeaderboardDefinitionCount")
	int32 EOK_Leaderboards_GetLeaderboardDefinitionCount();

	//Fetch the number of leaderboard records that are cached locally.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_GetLeaderboardRecordCount")
	int32 EOK_Leaderboards_GetLeaderboardRecordCount();

	//Fetch the number of leaderboard user scores that are cached locally.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_GetLeaderboardUserScoreCount")
	int32 EOK_Leaderboards_GetLeaderboardUserScoreCount();

	//Release the memory associated with leaderboard record. This must be called on data retrieved from EOS_Leaderboards_CopyLeaderboardRecordByIndex or EOS_Leaderboards_CopyLeaderboardRecordByUserId.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_LeaderboardRecord_Release")
	void EOK_Leaderboards_LeaderboardRecord_Release(FEOK_Leaderboards_LeaderboardRecord& LeaderboardRecord);

	//Release the memory associated with leaderboard user score. This must be called on data retrieved from EOS_Leaderboards_CopyLeaderboardUserScoreByIndex or EOS_Leaderboards_CopyLeaderboardUserScoreByUserId.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName="EOS_Leaderboards_LeaderboardUserScore_Release")
	void EOK_Leaderboards_LeaderboardUserScore_Release(FEOK_Leaderboards_LeaderboardUserScore& LeaderboardUserScore);
};
