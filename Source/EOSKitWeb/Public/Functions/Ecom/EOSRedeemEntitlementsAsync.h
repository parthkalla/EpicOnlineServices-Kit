// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSRedeemEntitlementsAsync.generated.h"

/**
 * Async node to redeem entitlements via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSRedeemEntitlementsAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Redeem entitlements
	 * @param Authorization - Bearer token for authentication
	 * @param IdentityId - Identity ID
	 * @param EntitlementIds - Array of entitlement IDs to redeem
	 * @param SandboxId - Sandbox ID
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Ecom",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Redeem Entitlements")
	static UEOSRedeemEntitlementsAsync* RedeemEntitlements(
		const FString& Authorization,
		const FString& IdentityId,
		const TArray<FString>& EntitlementIds,
		const FString& SandboxId
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_IdentityId;
	TArray<FString> Var_EntitlementIds;
	FString Var_SandboxId;
};

