// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSQueryAntiCheatServiceStatusAsync.generated.h"

/**
 * Async node to query AntiCheat service status via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSQueryAntiCheatServiceStatusAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Query AntiCheat service status
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID to query
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|AntiCheat",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Query AntiCheat Service Status")
	static UEOSQueryAntiCheatServiceStatusAsync* QueryAntiCheatServiceStatus(
		const FString& Authorization,
		const FString& DeploymentId
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
};

