// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSCreateSanctionAppealForLocalUserAsync.generated.h"

/**
 * Async node to create sanction appeal for local user via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSCreateSanctionAppealForLocalUserAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Create sanction appeal for local user
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param ProductUserId - Product User ID
	 * @param ReferenceId - Reference ID
	 * @param Reason - Reason for appeal
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Create Sanction Appeal For Local User")
	static UEOSCreateSanctionAppealForLocalUserAsync* CreateSanctionAppealForLocalUser(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& ProductUserId,
		const FString& ReferenceId,
		const FString& Reason
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_ProductUserId;
	FString Var_ReferenceId;
	FString Var_Reason;
};

