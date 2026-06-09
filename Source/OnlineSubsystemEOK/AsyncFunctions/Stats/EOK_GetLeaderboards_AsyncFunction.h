// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineLeaderboardsEOS.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_GetLeaderboards_AsyncFunction.generated.h"

USTRUCT(BlueprintType)
struct FEOKLeaderboardValue
{
	GENERATED_USTRUCT_BODY()
public:
	/** Name of player in this row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOKLeaderboardValue")
	FString NickName = "";
	/** Player's rank in this leaderboard */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOKLeaderboardValue")
	int32 Rank = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOKLeaderboardValue")
	int32 Score = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetLeaderboardResult, const TArray<FEOKLeaderboardValue>&, LeaderboardValues);

UCLASS()
class OnlineSubsystemEOK_API UEOK_GetLeaderboards_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	bool bDelegateCalled = false;
	
	UFUNCTION(BlueprintCallable, DisplayName="Get EOK Leaderboard", meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 | Stats")
	static UEOK_GetLeaderboards_AsyncFunction* GetLeaderboard(const FName LeaderboardName,const int32 Range, const int32 AroundRank);

	virtual void Activate() override;
	FName LeaderboardName;
	int32 Range;
	int32 AroundRank;
	void GetLeaderboardLocal();

	UPROPERTY(BlueprintAssignable)
	FGetLeaderboardResult OnSuccess;
	
	UPROPERTY(BlueprintAssignable)
	FGetLeaderboardResult OnFail;

	
	void OnGetLeaderboardCompleted(bool bWasSuccessful, FOnlineLeaderboardReadRef LeaderboardRead);
};
