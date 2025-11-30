// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "EOSGetAppleAuthTokenAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetAppleAuthTokenComplete, const FString&, AuthToken);

/**
 * Async node to get Apple auth token
 * Uses the Apple OnlineSubsystem's identity interface to retrieve auth tokens
 * If no token exists, attempts to login first
 */
UCLASS()
class EOSKITAUTH_API UEOSGetAppleAuthTokenAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Get Apple auth token
	 * @param WorldContextObject - World context
	 * @param LocalUserNum - Local user number (typically 0)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Auth|Extra",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Get Apple Auth Token")
	static UEOSGetAppleAuthTokenAsync* GetAppleAuthToken(
		UObject* WorldContextObject,
		int32 LocalUserNum = 0
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Auth|Extra")
	FOnGetAppleAuthTokenComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Auth|Extra")
	FOnGetAppleAuthTokenComplete OnFailure;

private:
	virtual void Activate() override;

	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UniqueNetId, const FString& Error);

	UPROPERTY()
	UObject* WorldContextObject;

	int32 LocalUserNum;
	FDelegateHandle LoginDelegateHandle;
};

