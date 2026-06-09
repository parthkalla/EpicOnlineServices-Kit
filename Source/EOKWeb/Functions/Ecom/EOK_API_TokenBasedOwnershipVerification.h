// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_TokenBasedOwnershipVerification.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_TokenBasedOwnershipVerification : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web API|AntiCheat")
	static UEOK_API_TokenBasedOwnershipVerification* TokenBasedOwnershipVerification(FString AuthorizationToken, FString Platform, FString IdentityId, TMap<FString, FString> CatalogItemId);

private:
	
	virtual void Activate() override;
	FString Var_Platform;
	FString Var_IdentityId;
	TMap<FString, FString> Var_CatalogItemId;
	FString Var_Authorization;
};
