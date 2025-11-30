// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSQueryAllSanctionsAsync.generated.h"

/**
 * Async node to query all sanctions via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSQueryAllSanctionsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Query all sanctions
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param Limit - Result limit (default: 100)
	 * @param Offset - Result offset (default: 0)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Query All Sanctions")
	static UEOSQueryAllSanctionsAsync* QueryAllSanctions(
		const FString& Authorization,
		const FString& DeploymentId,
		int32 Limit = 100,
		int32 Offset = 0
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	int32 Var_Limit;
	int32 Var_Offset;
};

