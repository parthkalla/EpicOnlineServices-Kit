// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSQueryAllSanctionsForPlayerAsync.generated.h"

/**
 * Async node to query all sanctions for a player via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSQueryAllSanctionsForPlayerAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Query all sanctions for a player
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param ProductUserId - Product User ID
	 * @param Limit - Result limit (default: 100)
	 * @param Offset - Result offset (default: 0)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Query All Sanctions For Player")
	static UEOSQueryAllSanctionsForPlayerAsync* QueryAllSanctionsForPlayer(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& ProductUserId,
		int32 Limit = 100,
		int32 Offset = 0
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_ProductUserId;
	int32 Var_Limit;
	int32 Var_Offset;
};

