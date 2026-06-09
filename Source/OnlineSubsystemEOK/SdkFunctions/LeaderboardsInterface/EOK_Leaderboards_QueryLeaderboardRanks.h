// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
THIRD_PARTY_INCLUDES_START
#include "eos_leaderboards.h"
#include "eos_leaderboards_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_Leaderboards_QueryLeaderboardRanks.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Leaderboards_QueryLeaderboardRanksOptions
{
	GENERATED_BODY()

	//The ID of the leaderboard whose information you want to retrieve.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface")
	FString LeaderboardId;

	//Product User ID for user who is querying ranks. Must be set when using a client policy that requires a valid logged in user. Not used for Dedicated Server where no user is available.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface")
	FEOK_ProductUserId LocalUserId;

	FEOK_Leaderboards_QueryLeaderboardRanksOptions()
		: LeaderboardId("")
		, LocalUserId()
	{
	}
	EOS_Leaderboards_QueryLeaderboardRanksOptions ToEOSLeaderboardsQueryLeaderboardRanksOptions()
	{
		EOS_Leaderboards_QueryLeaderboardRanksOptions Result;
		Result.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDRANKS_API_LATEST;
		Result.LeaderboardId = TCHAR_TO_ANSI(*LeaderboardId);
		Result.LocalUserId = LocalUserId.GetValueAsEosType();
		return Result;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Leaderboards_OnQueryLeaderboardRanksCompleteCallback, const TEnumAsByte<EEOK_Result>&, Result, const FString&, LeaderboardId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Leaderboards_QueryLeaderboardRanks : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Retrieves top leaderboard records by rank in the leaderboard matching the given leaderboard ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface", DisplayName = "EOS_Leaderboards_QueryLeaderboardRanks")
	static UEOK_Leaderboards_QueryLeaderboardRanks* EOK_Leaderboards_QueryLeaderboardRanks(const FEOK_Leaderboards_QueryLeaderboardRanksOptions& Options);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Leaderboards Interface")
	FEOK_Leaderboards_OnQueryLeaderboardRanksCompleteCallback OnCallback;

private:
	virtual void Activate() override;
	static void EOS_CALL Internal_OnQueryLeaderboardRanksCompleteCallback(const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo* Data);
	FEOK_Leaderboards_QueryLeaderboardRanksOptions Var_Options;
	
};
