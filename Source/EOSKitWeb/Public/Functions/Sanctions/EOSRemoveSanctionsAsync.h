// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSRemoveSanctionsAsync.generated.h"

/**
 * Async node to remove sanctions via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSRemoveSanctionsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Remove sanctions
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param ReferenceIds - Array of reference IDs to remove
	 * @param Justification - Justification for removal
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Sanctions",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Remove Sanctions")
	static UEOSRemoveSanctionsAsync* RemoveSanctions(
		const FString& Authorization,
		const FString& DeploymentId,
		const TArray<FString>& ReferenceIds,
		const FString& Justification
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	TArray<FString> Var_ReferenceIds;
	FString Var_Justification;
};

