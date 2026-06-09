// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_DirectEntitlementEnumeration.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_DirectEntitlementEnumeration : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web API|Ecom")
	static UEOK_API_DirectEntitlementEnumeration* DirectEntitlementEnumeration(FString Authorization, FString IdentityId, FString SandboxId, FString EntitlementName, bool bIncludeRedeemed);

private:
	
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_IdentityId;
	FString Var_SandboxId;
	FString Var_EntitlementName;
	bool Var_bIncludeRedeemed;
};
