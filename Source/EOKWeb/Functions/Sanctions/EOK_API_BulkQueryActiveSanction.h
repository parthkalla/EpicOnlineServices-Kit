// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_BulkQueryActiveSanction.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_BulkQueryActiveSanction : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_BulkQueryActiveSanction* BulkQueryActiveSanction(FString Authorization, FString DeploymentId, TArray<FString> ProductUserId, TArray<FString> Action);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	TArray<FString> Var_ProductUserId;
	TArray<FString> Var_Action;


};
