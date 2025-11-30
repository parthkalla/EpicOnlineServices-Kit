// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSGetReportReasonDefinitionAsync.generated.h"

/**
 * Async node to get report reason definition via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSGetReportReasonDefinitionAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Get report reason definition
	 * @param Authorization - Bearer token for authentication
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|PlayerReports",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Get Report Reason Definition")
	static UEOSGetReportReasonDefinitionAsync* GetReportReasonDefinition(
		const FString& Authorization
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
};

