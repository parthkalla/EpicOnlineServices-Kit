// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitEcomTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_ecom.h"
#include "eos_ecom_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// .generated.h must always be the last include
#include "EOSKitEcomSubsystem.generated.h"

/**
 * Ecom subsystem for managing EOS Ecom interface
 */
UCLASS()
class EOSKITECOM_API UEOSKitEcomSubsystem : public UGameInstanceSubsystem
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
	 * Query offers
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param OverrideCatalogNamespace - Optional catalog namespace override
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	EEOSResult QueryOffers(const FEOSKitEpicAccountId& LocalUserId, const FString& OverrideCatalogNamespace = TEXT(""));

	/**
	 * Query ownership for catalog items
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param CatalogItemIds - Array of catalog item IDs to check
	 * @param OverrideCatalogNamespace - Optional catalog namespace override
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	EEOSResult QueryOwnership(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& CatalogItemIds, const FString& OverrideCatalogNamespace = TEXT(""));

	/**
	 * Query ownership token
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param CatalogItemIds - Array of catalog item IDs to check
	 * @param OverrideCatalogNamespace - Optional catalog namespace override
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	EEOSResult QueryOwnershipToken(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& CatalogItemIds, const FString& OverrideCatalogNamespace = TEXT(""));

	/**
	 * Query entitlements
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param EntitlementNames - Array of entitlement names to filter (empty to get all)
	 * @param bIncludeRedeemed - If true, include redeemed entitlements
	 * @param OverrideCatalogNamespace - Optional catalog namespace override
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	EEOSResult QueryEntitlements(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& EntitlementNames, bool bIncludeRedeemed = false, const FString& OverrideCatalogNamespace = TEXT(""));

	/**
	 * Redeem entitlements
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param EntitlementIds - Array of entitlement IDs to redeem
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	EEOSResult RedeemEntitlements(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& EntitlementIds);

	/**
	 * Checkout (purchase offers)
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param OfferIds - Array of offer IDs to purchase
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	EEOSResult Checkout(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& OfferIds);

	/**
	 * Get offer count
	 * @param LocalUserId - Epic Account ID of the local user
	 * @return Number of offers (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	int32 GetOfferCount(const FEOSKitEpicAccountId& LocalUserId);

	/**
	 * Copy offer by index
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param Index - Index of the offer
	 * @param OutOffer - Output offer information
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	bool CopyOfferByIndex(const FEOSKitEpicAccountId& LocalUserId, int32 Index, FEOSKitCatalogOffer& OutOffer);

	/**
	 * Copy offer by ID
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param OfferId - Offer ID
	 * @param OutOffer - Output offer information
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	bool CopyOfferById(const FEOSKitEpicAccountId& LocalUserId, const FString& OfferId, FEOSKitCatalogOffer& OutOffer);

	/**
	 * Get entitlements count
	 * @param LocalUserId - Epic Account ID of the local user
	 * @return Number of entitlements (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	int32 GetEntitlementsCount(const FEOSKitEpicAccountId& LocalUserId);

	/**
	 * Copy entitlement by index
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param Index - Index of the entitlement
	 * @param OutEntitlement - Output entitlement information
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	bool CopyEntitlementByIndex(const FEOSKitEpicAccountId& LocalUserId, int32 Index, FEOSKitEntitlement& OutEntitlement);

	/**
	 * Copy entitlement by ID
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param EntitlementId - Entitlement ID
	 * @param OutEntitlement - Output entitlement information
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Ecom")
	bool CopyEntitlementById(const FEOSKitEpicAccountId& LocalUserId, const FString& EntitlementId, FEOSKitEntitlement& OutEntitlement);

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HEcom GetEcomHandle() const;
};

