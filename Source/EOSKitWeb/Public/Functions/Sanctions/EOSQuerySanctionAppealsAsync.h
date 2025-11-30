// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSQuerySanctionAppealsAsync.generated.h"

/**
 * Async node to query sanction appeals via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSQuerySanctionAppealsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Query sanction appeals
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param ReferenceId - Optional reference ID filter
	 * @param Status - Optional status filter
	 * @param ProductUserId - Optional Product User ID filter
	 * @param Limit - Result limit (default: 100)
	 * @param Offset - Result offset (default: 0)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Query Sanction Appeals")
	static UEOSQuerySanctionAppealsAsync* QuerySanctionAppeals(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& ReferenceId = TEXT(""),
		const FString& Status = TEXT(""),
		const FString& ProductUserId = TEXT(""),
		int32 Limit = 100,
		int32 Offset = 0
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_ReferenceId;
	FString Var_Status;
	FString Var_ProductUserId;
	int32 Var_Limit;
	int32 Var_Offset;
};

