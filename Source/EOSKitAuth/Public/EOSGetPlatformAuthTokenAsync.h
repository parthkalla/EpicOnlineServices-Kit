// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "EOSGetPlatformAuthTokenAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetPlatformAuthTokenComplete, const FString&, AuthToken);

/**
 * Async node to get platform auth token (e.g., Steam session ticket)
 * Uses the platform OnlineSubsystem's identity interface to retrieve linked account auth tokens
 */
UCLASS()
class EOSKITAUTH_API UEOSGetPlatformAuthTokenAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Get platform auth token
	 * @param WorldContextObject - World context
	 * @param LocalUserNum - Local user number (typically 0)
	 * @param TokenType - Optional token type (e.g., "Session" for Steam)
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Auth|Extra",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Get Platform Auth Token")
	static UEOSGetPlatformAuthTokenAsync* GetPlatformAuthToken(
		UObject* WorldContextObject,
		int32 LocalUserNum = 0,
		const FString& TokenType = TEXT("")
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Auth|Extra")
	FOnGetPlatformAuthTokenComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Auth|Extra")
	FOnGetPlatformAuthTokenComplete OnFailure;

private:
	virtual void Activate() override;

	void OnGetPlatformAuthTokenComplete(int32 InLocalUserNum, bool bWasSuccessful, const FExternalAuthToken& ExternalAuthToken);

	UPROPERTY()
	UObject* WorldContextObject;

	int32 LocalUserNum;
	FString TokenType;

	// Store platform identity for cleanup
	IOnlineIdentityPtr StoredPlatformIdentity;
	int32 StoredLocalUserNum;
};

