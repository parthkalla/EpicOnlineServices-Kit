// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSBulkQueryActiveSanctionAsync.generated.h"

/**
 * Async node to bulk query active sanctions via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSBulkQueryActiveSanctionAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Bulk query active sanctions
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param ProductUserIds - Array of Product User IDs to query
	 * @param Actions - Array of action filters
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Bulk Query Active Sanction")
	static UEOSBulkQueryActiveSanctionAsync* BulkQueryActiveSanction(
		const FString& Authorization,
		const FString& DeploymentId,
		const TArray<FString>& ProductUserIds,
		const TArray<FString>& Actions
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	TArray<FString> Var_ProductUserIds;
	TArray<FString> Var_Actions;
};

