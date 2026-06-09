// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_QueryAllSanctions.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_QueryAllSanctions : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_QueryAllSanctions* QueryAllSanctions(FString Authorization, FString DeploymentId, int32 Limit = 100, int32 Offset = 0);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	int32 Var_Limit;
	int32 Var_Offset;
};
