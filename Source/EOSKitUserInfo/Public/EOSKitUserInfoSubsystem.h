// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitUserInfoTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_userinfo.h"
#include "eos_userinfo_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// .generated.h must always be the last include
#include "EOSKitUserInfoSubsystem.generated.h"

/**
 * User Info subsystem for managing EOS UserInfo interface
 */
UCLASS()
class EOSKITUSERINFO_API UEOSKitUserInfoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// SDK Function Wrappers
	// ========================================

	/**
	 * Query user info for a target user
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param TargetUserId - Epic Account ID of the target user
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UserInfo")
	EEOSResult QueryUserInfo(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId);

	/**
	 * Query user info by external account
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param ExternalAccountId - External account ID
	 * @param AccountType - External account type
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UserInfo")
	EEOSResult QueryUserInfoByExternalAccount(const FEOSKitEpicAccountId& LocalUserId, 
		const FString& ExternalAccountId, 
		int32 AccountType);

	/**
	 * Get user info for a user (synchronous copy from cache)
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param TargetUserId - Epic Account ID of the target user
	 * @param OutUserInfo - Output user information
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UserInfo")
	bool GetUserInfo(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId, FEOSKitUserInfo& OutUserInfo);

	/**
	 * Get external account info count
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param TargetUserId - Epic Account ID of the target user
	 * @return Number of external accounts (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UserInfo")
	int32 GetExternalAccountInfoCount(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId);

	/**
	 * Get external account info at index
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param TargetUserId - Epic Account ID of the target user
	 * @param Index - Index of the external account
	 * @param OutExternalInfo - Output external account information
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UserInfo")
	bool GetExternalAccountInfoAtIndex(const FEOSKitEpicAccountId& LocalUserId, 
		const FEOSKitEpicAccountId& TargetUserId, 
		int32 Index, 
		FEOSKitExternalUserInfo& OutExternalInfo);

	/**
	 * Get external account info by account type
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param TargetUserId - Epic Account ID of the target user
	 * @param AccountType - External account type
	 * @param OutExternalInfo - Output external account information
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UserInfo")
	bool GetExternalAccountInfoByAccountType(const FEOSKitEpicAccountId& LocalUserId, 
		const FEOSKitEpicAccountId& TargetUserId, 
		int32 AccountType, 
		FEOSKitExternalUserInfo& OutExternalInfo);

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HUserInfo GetUserInfoHandle() const;
};

