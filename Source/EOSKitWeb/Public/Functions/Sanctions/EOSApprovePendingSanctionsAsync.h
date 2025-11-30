// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSApprovePendingSanctionsAsync.generated.h"

/**
 * Async node to approve pending sanctions via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSApprovePendingSanctionsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Approve pending sanctions
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param EOS_OrganizationId - EOS Organization ID
	 * @param EOS_ClientId - EOS Client ID
	 * @param EOS_DeploymentId - EOS Deployment ID
	 * @param ReferenceIds - Array of reference IDs to approve
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Approve Pending Sanctions")
	static UEOSApprovePendingSanctionsAsync* ApprovePendingSanctions(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& EOS_OrganizationId,
		const FString& EOS_ClientId,
		const FString& EOS_DeploymentId,
		const TArray<FString>& ReferenceIds
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_EOS_OrganizationId;
	FString Var_EOS_ClientId;
	FString Var_EOS_DeploymentId;
	TArray<FString> Var_ReferenceIds;
};

