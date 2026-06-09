// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_QueryAntiCheatServiceStatus.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_QueryAntiCheatServiceStatus : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web API|AntiCheat")
	static UEOK_API_QueryAntiCheatServiceStatus* QueryAntiCheatServiceStatus(FString Authorization, FString DeploymentId);

private:
	
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
};
