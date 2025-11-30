// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "eos_connect.h"
#include "eos_connect_types.h"
#include "EOSDeleteDeviceIdAsync.generated.h"

/**
 * Delegate for device ID deletion completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEOSDeleteDeviceIdComplete);

/**
 * Async node to delete a device ID
 * This removes the device ID authentication for the current user
 * Useful for resetting authentication state or allowing re-authentication
 */
UCLASS()
class EOSKITAUTH_API UEOSDeleteDeviceIdAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Auth")
	FOnEOSDeleteDeviceIdComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Auth")
	FOnEOSDeleteDeviceIdComplete OnFailure;

	/**
	 * Delete device ID for the current user
	 * @param WorldContextObject - World context object
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Delete EOS Device ID",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Delete the device ID authentication. This removes the device ID for the current user and allows re-authentication."),
		Category = "EOSKit|Auth")
	static UEOSDeleteDeviceIdAsync* DeleteDeviceId(UObject* WorldContextObject);

	virtual void Activate() override;

private:
	static void EOS_CALL OnDeleteDeviceIdComplete(const EOS_Connect_DeleteDeviceIdCallbackInfo* Data);

	UObject* WorldContextObject;
};

