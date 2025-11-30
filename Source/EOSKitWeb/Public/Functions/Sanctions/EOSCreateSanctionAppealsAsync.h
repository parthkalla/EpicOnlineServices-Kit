// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSCreateSanctionAppealsAsync.generated.h"

/**
 * Async node to create sanction appeals via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSCreateSanctionAppealsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Create sanction appeals
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param ReferenceId - Reference ID
	 * @param Reason - Reason ID (integer)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Create Sanction Appeals")
	static UEOSCreateSanctionAppealsAsync* CreateSanctionAppeals(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& ReferenceId,
		int32 Reason
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_ReferenceId;
	int32 Var_Reason;
};

