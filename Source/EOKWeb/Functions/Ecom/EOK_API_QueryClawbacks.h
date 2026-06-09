// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_QueryClawbacks.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_QueryClawbacks : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web API|Ecom")
	static UEOK_API_QueryClawbacks* QueryClawbacks(FString Authorization, FString NameSpace, FString ClawbackDate, int32 Count=10);

private:
	
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_NameSpace;
	FString Var_ClawbackDate;
	int32 Var_Count;
};
