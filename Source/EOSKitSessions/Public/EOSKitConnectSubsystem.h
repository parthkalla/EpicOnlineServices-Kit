// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSessionStructs.h"
#include "EOSKitConnectSubsystem.generated.h"

// Forward declarations
class UEOSKitSubsystem;

/**
 * Delegate for auth expiration notifications
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FEOSKitOnAuthExpiration, FString, ProductUserId);

/**
 * Delegate for login status change notifications
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKitOnLoginStatusChanged, FString, ProductUserId, int32, LoginStatus);

/**
 * Login status enum
 */
UENUM(BlueprintType)
enum class EEOSKitLoginStatus : uint8
{
	EKLS_NotLoggedIn UMETA(DisplayName = "Not Logged In"),
	EKLS_UsingLocalProfile UMETA(DisplayName = "Using Local Profile"),
	EKLS_LoggedIn UMETA(DisplayName = "Logged In")
};

/**
 * External Account Info structure
 */
USTRUCT(BlueprintType)
struct FEOSKitExternalAccountInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Connect")
	FString ProductUserId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Connect")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Connect")
	FString AccountId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Connect")
	EEOSKitExternalAccountType AccountIdType;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Connect")
	FString LastLoginTime;

	FEOSKitExternalAccountInfo()
		: ProductUserId(TEXT(""))
		, DisplayName(TEXT(""))
		, AccountId(TEXT(""))
		, AccountIdType(EEOSKitExternalAccountType::EKAT_Epic)
		, LastLoginTime(TEXT(""))
	{
	}
};

/**
 * EOSKit Connect Subsystem
 * 
 * Manages EOS Connect interface operations including:
 * - Auth expiration monitoring
 * - Login status tracking
 * - External account management
 * - Product User ID mappings
 */
UCLASS()
class EOSKITSESSIONS_API UEOSKitConnectSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// Notification Management
	// ========================================

	/**
	 * Register for auth expiration notifications
	 * Notification sent ~10 minutes before token expiration
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	bool RegisterAuthExpirationNotification(const FEOSKitOnAuthExpiration& Callback);

	/**
	 * Register for login status change notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	bool RegisterLoginStatusChangedNotification(const FEOSKitOnLoginStatusChanged& Callback);

	/**
	 * Unregister from all connect notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	void UnregisterAllNotifications();

	// ========================================
	// ID Token Management
	// ========================================

	/**
	 * Copy ID token for a Product User ID
	 * 
	 * @param ProductUserId - User to get token for
	 * @param OutIdToken - Retrieved ID token
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	bool CopyIdToken(const FString& ProductUserId, FEOSKitConnectIdToken& OutIdToken);

	// ========================================
	// External Account Management
	// ========================================

	/**
	 * Get external account info by account ID
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	bool CopyProductUserExternalAccountByAccountId(
		const FString& ProductUserId,
		const FString& AccountId,
		FEOSKitExternalAccountInfo& OutAccountInfo
	);

	/**
	 * Get external account info by account type
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	bool CopyProductUserExternalAccountByAccountType(
		const FString& ProductUserId,
		EEOSKitExternalAccountType AccountType,
		FEOSKitExternalAccountInfo& OutAccountInfo
	);

	/**
	 * Get external account info by index
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	bool CopyProductUserExternalAccountByIndex(
		const FString& ProductUserId,
		int32 Index,
		FEOSKitExternalAccountInfo& OutAccountInfo
	);

	/**
	 * Get product user info (most recent login account)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	bool CopyProductUserInfo(
		const FString& ProductUserId,
		FEOSKitExternalAccountInfo& OutAccountInfo
	);

	// ========================================
	// Account Mapping
	// ========================================

	/**
	 * Get Product User ID from external account ID
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	FString GetExternalAccountMapping(
		const FString& LocalUserId,
		EEOSKitExternalAccountType AccountType,
		const FString& TargetExternalUserId
	);

	/**
	 * Get external account ID from Product User ID
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	bool GetProductUserIdMapping(
		const FString& LocalUserId,
		EEOSKitExternalAccountType AccountType,
		const FString& TargetUserId,
		FString& OutExternalAccountId
	);

	// ========================================
	// User Status Queries
	// ========================================

	/**
	 * Get number of logged in users
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOSKit|Connect|Subsystem")
	int32 GetLoggedInUsersCount() const;

	/**
	 * Get logged in user by index
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	FString GetLoggedInUserByIndex(int32 Index) const;

	/**
	 * Get login status for a user
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	EEOSKitLoginStatus GetLoginStatus(const FString& ProductUserId) const;

	/**
	 * Get count of external accounts linked to Product User ID
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Connect|Subsystem")
	int32 GetProductUserExternalAccountCount(const FString& ProductUserId) const;

private:
	// Get EOSKit Subsystem
	UEOSKitSubsystem* GetEOSKitSubsystem() const;

	// Static callback functions
	static void OnAuthExpirationCallback(const void* Data);
	static void OnLoginStatusChangedCallback(const void* Data);

	// Notification delegates
	FEOSKitOnAuthExpiration OnAuthExpirationDelegate;
	FEOSKitOnLoginStatusChanged OnLoginStatusChangedDelegate;

	// Notification IDs
	uint64 AuthExpirationNotificationId = 0;
	uint64 LoginStatusChangedNotificationId = 0;
};
