// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_leaderboards.h"
#include "eos_leaderboards_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_Leaderboards_QueryLeaderboardDefinitions.generated.h"

//Input parameters for the EOS_Leaderboards_QueryLeaderboardDefinitions function. StartTime and EndTime are optional parameters, they can be used to limit the list of definitions to overlap the time window specified.
USTRUCT(BlueprintType)
struct FEOK_Leaderboards_QueryLeaderboardDefinitionsOptions
{
	GENERATED_BODY()

	//An optional POSIX timestamp for the leaderboard's start time, or EOS_LEADERBOARDS_TIME_UNDEFINED
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface")
	int64 StartTime;

	//An optional POSIX timestamp for the leaderboard's end time, or EOS_LEADERBOARDS_TIME_UNDEFINED
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface")
	int64 EndTime;

	//Product User ID for user who is querying definitions. Must be set when using a client policy that requires a valid logged in user. Not used for Dedicated Server where no user is available.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface")
	FEOK_ProductUserId LocalUserId;

	FEOK_Leaderboards_QueryLeaderboardDefinitionsOptions()
		: StartTime(EOS_LEADERBOARDS_TIME_UNDEFINED)
		, EndTime(EOS_LEADERBOARDS_TIME_UNDEFINED)
		, LocalUserId()
	{
	}
	EOS_Leaderboards_QueryLeaderboardDefinitionsOptions ToEOSLeaderboardsQueryLeaderboardDefinitionsOptions()
	{
		EOS_Leaderboards_QueryLeaderboardDefinitionsOptions Result;
		Result.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST;
		Result.StartTime = StartTime;
		Result.EndTime = EndTime;
		Result.LocalUserId = LocalUserId.GetValueAsEosType();
		return Result;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback, const TEnumAsByte<EEOK_Result>&, Result);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Leaderboards_QueryLeaderboardDefinitions : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName = "EOS_Leaderboards_QueryLeaderboardDefinitions")
	static UEOK_Leaderboards_QueryLeaderboardDefinitions* EOK_Leaderboards_QueryLeaderboardDefinitions(const FEOK_Leaderboards_QueryLeaderboardDefinitionsOptions& Options);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface")
	FEOK_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback OnCallback;

private:
	FEOK_Leaderboards_QueryLeaderboardDefinitionsOptions Var_Options;
	static void EOS_CALL OnQueryLeaderboardDefinitionsCompleteCallback(const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo* Data);
	virtual void Activate() override;
};
