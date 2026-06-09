// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_VerifyOwnershipVerificationToken.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_VerifyOwnershipVerificationToken : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web API|Ecom")
	static UEOK_API_VerifyOwnershipVerificationToken* VerifyOwnershipVerificationToken(FString KeyId);

private:
	
	virtual void Activate() override;
	FString Var_KeyId;
};
