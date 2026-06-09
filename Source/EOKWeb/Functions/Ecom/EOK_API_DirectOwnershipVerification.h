// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_DirectOwnershipVerification.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_DirectOwnershipVerification : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:
	//The endpoint to check if a user owns an item, or a list of items.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web API|AntiCheat")
	static UEOK_API_DirectOwnershipVerification* DirectOwnershipVerification(FString Authorization, FString CurrentAccountId, TArray<FString> NsCatalogItemIds, FString SandboxId);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_CurrentAccountId;
	TArray<FString> Var_NsCatalogItemIds;
	FString Var_SandboxId;
};
