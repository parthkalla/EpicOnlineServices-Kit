// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionStructs.h"
#include "EOSCreateDeviceIdAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateDeviceId_Delegate, bool, bSuccess);

/**
 * Create Device ID - Create a unique pseudo-account for the local device
 * 
 * This allows users to start playing without login and preserves progress
 * when they later link a real account.
 */
UCLASS()
class EOSKITSESSIONS_API UEOSCreateDeviceIdAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, DisplayName="On Complete")
	FCreateDeviceId_Delegate OnComplete;

	/**
	 * Create a new unique pseudo-account for the local device
	 * 
	 * Use Cases:
	 * - Mobile games allowing play without login
	 * - PC games with guest accounts
	 * - Automatic login on subsequent starts
	 * 
	 * Benefits:
	 * - No login required to start playing
	 * - Progress preserved when linking real account
	 * - Multiple devices can link to same account
	 * 
	 * Important:
	 * - Returns EOS_DuplicateNotAllowed if Device ID already exists
	 * - After creation, use EOS_ECT_DEVICEID_ACCESS_TOKEN for login
	 * - Link a real account to prevent data loss if device is lost
	 * 
	 * @param DeviceModel - Device model identifier (e.g., "iPhone14,3", "SM-G998B")
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Create Device ID", meta = (BlueprintInternalUseOnly = "true"), Category="EOSKit|Sessions")
	static UEOSCreateDeviceIdAsync* CreateDeviceId(FString DeviceModel);

	virtual void Activate() override;

private:
	void CreateDevice();
	
	FString VarDeviceModel;
};
