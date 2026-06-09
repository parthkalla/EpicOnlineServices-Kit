// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_RedeemEntitlements.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_RedeemEntitlements : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web API|Ecom")
	static UEOK_API_RedeemEntitlements* RedeemEntitlements(FString Authorization, FString IdentityId, TArray<FString> EntitlementIds, FString SandboxId);

private:
	
	virtual void Activate() override;
	FString Var_IdentityId;
	FString Var_SandboxId;
	FString Var_Authorization;
	TArray<FString> Var_EntitlementIds;
};
