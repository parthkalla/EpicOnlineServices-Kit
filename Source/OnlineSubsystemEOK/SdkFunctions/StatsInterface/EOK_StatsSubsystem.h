// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_stats.h"
#include "eos_stats_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_StatsSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class OnlineSubsystemEOK_API UEOK_StatsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//Fetches a stat from a given index. Use EOS_Stats_Stat_Release when finished with the data.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface", DisplayName="EOS_Stats_CopyStatByIndex")
	static TEnumAsByte<EEOK_Result> EOK_Stats_CopyStatByIndex( FEOK_ProductUserId TargetUserId, int32 StatIndex, FEOK_Stats_Stat& OutStat);

	//Fetches a stat from cached stats by name. Use EOS_Stats_Stat_Release when finished with the data.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface", DisplayName="EOS_Stats_CopyStatByName")
	static TEnumAsByte<EEOK_Result> EOK_Stats_CopyStatByName( FEOK_ProductUserId TargetUserId, const FString& Name, FEOK_Stats_Stat& OutStat);

	//Fetch the number of stats that are cached locally.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface", DisplayName="EOS_Stats_GetStatsCount")
	static int32 EOK_Stats_GetStatsCount( FEOK_ProductUserId TargetUserId);

};
