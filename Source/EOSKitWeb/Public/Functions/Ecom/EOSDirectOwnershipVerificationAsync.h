// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSDirectOwnershipVerificationAsync.generated.h"

/**
 * Async node to verify ownership directly via Web API
 * Checks if a user owns an item or list of items
 */
UCLASS()
class EOSKITWEB_API UEOSDirectOwnershipVerificationAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Direct ownership verification
	 * @param Authorization - Bearer token for authentication
	 * @param CurrentAccountId - Current account ID
	 * @param NsCatalogItemIds - Array of catalog item IDs (without sandbox prefix)
	 * @param SandboxId - Sandbox ID (will be prefixed to catalog item IDs)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Ecom",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Direct Ownership Verification")
	static UEOSDirectOwnershipVerificationAsync* DirectOwnershipVerification(
		const FString& Authorization,
		const FString& CurrentAccountId,
		const TArray<FString>& NsCatalogItemIds,
		const FString& SandboxId
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_CurrentAccountId;
	TArray<FString> Var_NsCatalogItemIds;
	FString Var_SandboxId;
};

