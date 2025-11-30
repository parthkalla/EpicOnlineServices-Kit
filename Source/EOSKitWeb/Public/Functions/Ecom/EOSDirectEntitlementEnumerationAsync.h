// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSDirectEntitlementEnumerationAsync.generated.h"

/**
 * Async node to enumerate entitlements directly via Web API
 */
UCLASS()
class EOSKITWEB_API UEOSDirectEntitlementEnumerationAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Direct entitlement enumeration
	 * @param Authorization - Bearer token for authentication
	 * @param IdentityId - Identity ID
	 * @param SandboxId - Sandbox ID
	 * @param EntitlementName - Optional entitlement name filter
	 * @param bIncludeRedeemed - Whether to include redeemed entitlements
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Ecom",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Direct Entitlement Enumeration")
	static UEOSDirectEntitlementEnumerationAsync* DirectEntitlementEnumeration(
		const FString& Authorization,
		const FString& IdentityId,
		const FString& SandboxId,
		const FString& EntitlementName = TEXT(""),
		bool bIncludeRedeemed = false
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_IdentityId;
	FString Var_SandboxId;
	FString Var_EntitlementName;
	bool Var_bIncludeRedeemed;
};

