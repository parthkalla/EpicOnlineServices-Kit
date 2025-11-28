// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSLinkAccountAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLinkAccount_Delegate, bool, bSuccess);

/**
 * Link Account - Link external account with existing Product User ID
 * 
 * This allows linking authentication methods to preserve game progression
 * across platforms and prevent accidental new account creation.
 */
UCLASS()
class EOSKITSESSIONS_API UEOSLinkAccountAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FLinkAccount_Delegate OnComplete;

	/**
	 * Link an external account with an existing Product User ID
	 * 
	 * Use Cases:
	 * - User logged in with Device ID, wants to link Epic/Steam account
	 * - Preserve game progression when switching platforms
	 * - Prevent accidental new account creation
	 * - Cross-platform account unification
	 * 
	 * Flow:
	 * 1. User attempts login with external account (Epic/Steam/etc.)
	 * 2. Login fails with EOS_InvalidUser + Continuance Token
	 * 3. Call LinkAccount with token to link external account
	 * 4. User can now login with either Device ID or external account
	 * 
	 * @param LocalProductUserId - Product User ID to link account to (from Device ID)
	 * @param ContinuanceToken - Token from failed login attempt
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Link Account", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSLinkAccountAsync* LinkAccount(
		FString LocalProductUserId,
		FEOSKitContinuanceToken ContinuanceToken
	);

	virtual void Activate() override;

private:
	void PerformLink();
	
	FString VarLocalProductUserId;
	FEOSKitContinuanceToken VarContinuanceToken;
};
