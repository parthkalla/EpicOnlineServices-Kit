// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_QueryActiveSanctions.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_QueryActiveSanctions : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	//The queryActiveSanctions request will return a list of all active sanctions for a given player.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_QueryActiveSanctions* QueryActiveSanctions(FString Authorization, FString ProductUserId, TArray<FString> Action);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_ProductUserId;
	TArray<FString> Var_Action;
};
