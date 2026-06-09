// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_RemoveSanctions.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_RemoveSanctions : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_RemoveSanctions* RemoveSanctions(FString Authorization, FString DeploymentId, TArray<FString> ReferenceIds, FString Justification);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	TArray<FString> Var_ReferenceIds;
	FString Var_Justification;
};
