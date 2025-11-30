// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "eos_auth.h"
#include "eos_auth_types.h"
#include "EOSGetIdTokenAsync.generated.h"

/**
 * Structure containing ID token information
 */
USTRUCT(BlueprintType, Category = "EOSKit|Auth")
struct EOSKITAUTH_API FEOSKitIdToken
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Auth")
	FString EpicAccountId = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Auth")
	FString IdToken_JWT = TEXT("");
};

/**
 * Delegate for ID token retrieval completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSGetIdTokenComplete, const FEOSKitIdToken&, IdToken);

/**
 * Async node to retrieve an ID token for an Epic Account ID
 * ID tokens are used to securely verify user identities with online services
 * Returns an ID token as a JSON Web Token (JWT) in string format
 * Only works with Epic accounts
 */
UCLASS()
class EOSKITAUTH_API UEOSGetIdTokenAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Auth")
	FOnEOSGetIdTokenComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Auth")
	FOnEOSGetIdTokenComplete OnFailure;

	/**
	 * Get ID token for an Epic Account ID
	 * @param WorldContextObject - World context object
	 * @param EpicAccountId - Epic Account ID string
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get EOS ID Token", 
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Get ID token, often used for OpenID. Returns an ID token as a JSON Web Token (JWT) in string format. Only works with Epic accounts."), 
		Category = "EOSKit|Auth")
	static UEOSGetIdTokenAsync* GetIdToken(UObject* WorldContextObject, const FString& EpicAccountId);

	virtual void Activate() override;

private:
	UObject* WorldContextObject;
	FString EpicAccountIdString;
	
	void GetIdTokenInternal();
};

