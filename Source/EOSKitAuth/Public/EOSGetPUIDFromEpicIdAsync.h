// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitUserInfoTypes.h"
#include "EOSGetPUIDFromEpicIdAsync.generated.h"

/**
 * Delegate for get PUID from Epic ID completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSGetPUIDFromEpicIdComplete, const TArray<FEOSKitProductUserIdAndEpicId>&, UserInfo);

/**
 * Async node to get Product User ID from Epic Account ID
 * Uses Connect interface to query external account mappings
 */
UCLASS()
class EOSKITAUTH_API UEOSGetPUIDFromEpicIdAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|UserInfo")
	FOnEOSGetPUIDFromEpicIdComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|UserInfo")
	FOnEOSGetPUIDFromEpicIdComplete OnFailure;

	/**
	 * Get Product User ID from Epic Account ID(s)
	 * @param WorldContextObject - World context object
	 * @param TargetEpicAccountIds - Array of Epic Account ID strings
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get Product User ID From Epic ID",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Get Product User ID(s) from Epic Account ID(s) using Connect interface"),
		Category = "EOSKit|UserInfo")
	static UEOSGetPUIDFromEpicIdAsync* GetPUIDFromEpicId(UObject* WorldContextObject, const TArray<FString>& TargetEpicAccountIds);

	virtual void Activate() override;

private:
	static void EOS_CALL OnQueryExternalAccountMappingsComplete(const EOS_Connect_QueryExternalAccountMappingsCallbackInfo* Data);

	UObject* WorldContextObject;
	TArray<FString> TargetEpicAccountIdStrings;
};

