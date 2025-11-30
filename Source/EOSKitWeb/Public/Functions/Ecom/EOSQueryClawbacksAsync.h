// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSQueryClawbacksAsync.generated.h"

/**
 * Async node to query clawbacks via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSQueryClawbacksAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Query clawbacks
	 * @param Authorization - Bearer token for authentication
	 * @param NameSpace - Namespace
	 * @param ClawbackDate - Clawback date
	 * @param Count - Number of results to return (default: 10)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Ecom",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Query Clawbacks")
	static UEOSQueryClawbacksAsync* QueryClawbacks(
		const FString& Authorization,
		const FString& NameSpace,
		const FString& ClawbackDate,
		int32 Count = 10
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_NameSpace;
	FString Var_ClawbackDate;
	int32 Var_Count;
};

