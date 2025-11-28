// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSUnlinkAccountAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUnlinkAccount_Delegate, const FString&, LocalUserId, bool, bSuccess);

/**
 * Unlink Account - Unlink external auth credentials from the owning keychain
 * 
 * This allows recovering from accidental new account creation or separating accounts
 * that were previously linked together.
 */
UCLASS()
class EOSKITSESSIONS_API UEOSUnlinkAccountAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FUnlinkAccount_Delegate OnComplete;

	/**
	 * Unlink external auth credentials from the owning keychain of a logged in product user
	 * 
	 * Use Cases:
	 * 1. User accidentally created new account instead of linking with existing one
	 * 2. User wants to disassociate an account from current keychain
	 * 3. User wants to link account to different keychain
	 * 
	 * Security: Can only unlink accounts authenticated in current session
	 * 
	 * @param LocalUserId - Product User ID of the local user
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Unlink Account", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSUnlinkAccountAsync* UnlinkAccount(FString LocalUserId);

	virtual void Activate() override;

private:
	void PerformUnlink();
	
	FString VarLocalUserId;
};
