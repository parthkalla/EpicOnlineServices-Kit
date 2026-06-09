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
#include "EOK_Stats_IngestStat.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Stats_IngestData
{
	GENERATED_BODY()

	/** The name of the stat to ingest. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface")
	FString Name;

	/** The amount to ingest the stat. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface")
	int32 IngestAmount;

	FEOK_Stats_IngestData()
		: Name("")
		, IngestAmount(0)
	{}
	EOS_Stats_IngestData ToEOSStatsIngestData()
	{
		EOS_Stats_IngestData Data;
		Data.ApiVersion = EOS_STATS_INGESTDATA_API_LATEST;
		Data.StatName = TCHAR_TO_ANSI(*Name);
		Data.IngestAmount = IngestAmount;
		return Data;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_OnStatsIngestStatComplete, const FEOK_ProductUserId&, LocalUserId, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_ProductUserId&, TargetUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Stats_IngestStat : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	
public:
	
	//Ingest a stat by the amount specified in Options. When the operation is complete and the delegate is triggered the stat will be uploaded to the backend to be processed. The stat may not be updated immediately and an achievement using the stat may take a while to be unlocked once the stat has been uploaded.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface", DisplayName="EOS_Stats_IngestStat")
	static UEOK_Stats_IngestStat* EOK_Stats_IngestStat(FEOK_ProductUserId LocalUserId, const TArray<FEOK_Stats_IngestData>& Stats, const FEOK_ProductUserId& TargetUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Stats Interface")
	FEOK_OnStatsIngestStatComplete OnCallback;

private:
	FEOK_ProductUserId Var_LocalUserId;
	TArray<FEOK_Stats_IngestData> Var_Stats;
	FEOK_ProductUserId Var_TargetUserId;
	virtual void Activate() override;
	static void EOS_CALL Internal_OnStatsIngestStatComplete(const EOS_Stats_IngestStatCompleteCallbackInfo* Data);
};
