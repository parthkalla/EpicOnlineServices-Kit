// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitEcomTypes.generated.h"

/**
 * Item ownership information
 */
USTRUCT(BlueprintType, Category = "EOSKit|Ecom")
struct EOSKITECOM_API FEOSKitItemOwnership
{
	GENERATED_BODY()

	/** ID of the catalog item */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString CatalogItemId = TEXT("");

	/** Is this catalog item owned by the local user */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	bool bIsOwned = false;
};

/**
 * Catalog offer information
 */
USTRUCT(BlueprintType, Category = "EOSKit|Ecom")
struct EOSKITECOM_API FEOSKitCatalogOffer
{
	GENERATED_BODY()

	/** The ID of this offer */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString OfferId = TEXT("");

	/** Product namespace in which this offer exists */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString CatalogNamespace = TEXT("");

	/** Localized UTF-8 title of this offer */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString TitleText = TEXT("");

	/** Localized UTF-8 description of this offer */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString DescriptionText = TEXT("");

	/** Localized UTF-8 long description of this offer */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString LongDescriptionText = TEXT("");

	/** The Currency Code for this offer */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString CurrencyCode = TEXT("");

	/** The original price of this offer (in lowest denomination, e.g., cents) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int64 OriginalPrice64 = 0;

	/** The current price including discounts (in lowest denomination, e.g., cents) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int64 CurrentPrice64 = 0;

	/** The decimal point for the provided price */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int32 DecimalPoint = 0;

	/** A value from 0 to 100 define the percentage of the OriginalPrice that the CurrentPrice represents */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int32 DiscountPercentage = 0;

	/** Contains the POSIX timestamp that the offer expires or -1 if it does not expire */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int64 ExpirationTimestamp = -1;

	/** The maximum number of times that the offer can be purchased. A negative value implies there is no limit. */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int32 PurchaseLimit = -1;

	/** True if the user can purchase this offer */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	bool bAvailableForPurchase = false;

	/** Timestamp indicating when the time when the offer was released */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int64 ReleaseDateTimestamp = -1;

	/** Timestamp indicating the effective date of the offer */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int64 EffectiveDateTimestamp = -1;
};

/**
 * Entitlement information
 */
USTRUCT(BlueprintType, Category = "EOSKit|Ecom")
struct EOSKITECOM_API FEOSKitEntitlement
{
	GENERATED_BODY()

	/** Name of the entitlement */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString EntitlementName = TEXT("");

	/** ID of the entitlement owned by an account */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString EntitlementId = TEXT("");

	/** ID of the item associated with the offer which granted this entitlement */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	FString CatalogItemId = TEXT("");

	/** If true then the catalog has this entitlement marked as redeemed */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	bool bRedeemed = false;

	/** If not -1 then this is a POSIX timestamp that this entitlement will end */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Ecom")
	int64 EndTimestamp = -1;
};

