// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_QueryProductUsers.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_QueryProductUsers : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	//The request queryProductUsersForAnyUser returns associated accounts from a list of Product User IDs.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web API|Connect")
	static UEOK_API_QueryProductUsers* QueryProductUsers(FString Authorization, TArray<FString> ProductUserIds);

private:
	
	virtual void Activate() override;
	FString Var_Authorization;
	TArray<FString> Var_ProductUserIds;
};
