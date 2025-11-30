// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSFindPlayerReportsAsync.generated.h"

/**
 * Async node to find player reports via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSFindPlayerReportsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Find player reports
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param ReportingPlayerId - Optional reporting player ID filter
	 * @param ReportedPlayerId - Optional reported player ID filter
	 * @param ReasonId - Optional reason ID filter (0 to ignore)
	 * @param StartTime - Optional start time filter
	 * @param EndTime - Optional end time filter
	 * @param bPagination - Whether to use pagination
	 * @param Offset - Pagination offset (default: 0)
	 * @param Limit - Pagination limit (default: 50)
	 * @param Order - Sort order (default: "time:desc")
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|PlayerReports",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Find Player Reports")
	static UEOSFindPlayerReportsAsync* FindPlayerReports(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& ReportingPlayerId = TEXT(""),
		const FString& ReportedPlayerId = TEXT(""),
		int32 ReasonId = 0,
		const FString& StartTime = TEXT(""),
		const FString& EndTime = TEXT(""),
		bool bPagination = false,
		int32 Offset = 0,
		int32 Limit = 50,
		const FString& Order = TEXT("time:desc")
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_ReportingPlayerId;
	FString Var_ReportedPlayerId;
	int32 Var_ReasonId;
	FString Var_StartTime;
	FString Var_EndTime;
	bool Var_bPagination;
	int32 Var_Offset;
	int32 Var_Limit;
	FString Var_Order;
};

