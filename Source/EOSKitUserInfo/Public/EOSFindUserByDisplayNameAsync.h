// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitUserInfoTypes.h"
#include "EOSFindUserByDisplayNameAsync.generated.h"

/**
 * Delegate for find user by display name completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSFindUserByDisplayNameComplete, const FEOSKitUserInfo&, UserInfo);

/**
 * Async node to find user by display name
 * Queries user information by display name and returns Epic Account ID and user info
 */
UCLASS()
class EOSKITUSERINFO_API UEOSFindUserByDisplayNameAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|UserInfo")
	FOnEOSFindUserByDisplayNameComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|UserInfo")
	FOnEOSFindUserByDisplayNameComplete OnFailure;

	/**
	 * Find user by display name
	 * @param WorldContextObject - World context object
	 * @param TargetDisplayName - Display name to search for
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Find User By Display Name",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Find user information by display name. Returns Epic Account ID and user info."),
		Category = "EOSKit|UserInfo")
	static UEOSFindUserByDisplayNameAsync* FindUserByDisplayName(UObject* WorldContextObject, const FString& TargetDisplayName);

	virtual void Activate() override;

private:
	static void EOS_CALL OnQueryUserInfoByDisplayNameComplete(const EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo* Data);

	UObject* WorldContextObject;
	FString TargetDisplayNameString;
};

