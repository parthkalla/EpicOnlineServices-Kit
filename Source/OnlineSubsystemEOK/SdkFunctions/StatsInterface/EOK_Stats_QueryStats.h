// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_stats.h"
#include "eos_stats_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_Stats_QueryStats.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_OnStatsQueryStatsComplete, const FEOK_ProductUserId&, LocalUserId, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_ProductUserId&, TargetUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Stats_QueryStats : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Query for a list of stats for a specific player.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface", DisplayName="EOS_Stats_QueryStats")
	static UEOK_Stats_QueryStats* EOK_Stats_QueryStats(FEOK_ProductUserId LocalUserId, const FEOK_ProductUserId& TargetUserId, int64 StartTime, int64 EndTime, const TArray<FString>& StatNames);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface")
	FEOK_OnStatsQueryStatsComplete OnCallback;

private:
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_ProductUserId Var_TargetUserId;
	int64 Var_StartTime;
	int64 Var_EndTime;
	TArray<FString> Var_StatNames;

	static void EOS_CALL Internal_OnStatsQueryStatsComplete(const EOS_Stats_OnQueryStatsCompleteCallbackInfo* Data);
	void Activate() override;
};
