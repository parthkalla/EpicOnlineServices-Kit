// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_CreateSanctionAppealForLocalUser.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_CreateSanctionAppealForLocalUser : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_CreateSanctionAppealForLocalUser* CreateSanctionAppealForLocalUser(FString Authorization, FString DeploymentId, FString ProductUserId, FString ReferenceId, FString Reason);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_ProductUserId;
	FString Var_ReferenceId;
	FString Var_Reason;
};
