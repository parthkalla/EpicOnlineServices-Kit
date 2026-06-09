// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_EcomSubsystem.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="Ecom Interface"), Category="Epic Online Services-Kit V2", DisplayName="Ecom Interface")
class OnlineSubsystemEOK_API UEOK_EcomSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	//Release the memory associated with an EOS_Ecom_CatalogItem structure. This must be called on data retrieved from EOS_Ecom_CopyOfferItemByIndex.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CatalogItem_Release")
	void EOK_Ecom_CatalogItem_Release(FEOK_Ecom_CatalogItem CatalogItem);

	//Release the memory associated with an EOS_Ecom_CatalogOffer structure. This must be called on data retrieved from EOS_Ecom_CopyOfferByIndex.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CatalogOffer_Release")
	void EOK_Ecom_CatalogOffer_Release(FEOK_Ecom_CatalogOffer CatalogOffer);

	//Release the memory associated with an EOS_Ecom_CatalogRelease structure. This must be called on data retrieved from EOS_Ecom_CopyItemReleaseByIndex.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CatalogRelease_Release")
	void EOK_Ecom_CatalogRelease_Release(FEOK_Ecom_CatalogRelease CatalogRelease);

	//Fetches the entitlement with the given ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyEntitlementById")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyEntitlementById(FEOK_EpicAccountId LocalUserId, const FString& EntitlementId, FEOK_Ecom_Entitlement& OutEntitlement);

	//Fetches an entitlement from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyEntitlementByIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyEntitlementByIndex(FEOK_EpicAccountId LocalUserId, int32 EntitlementIndex, FEOK_Ecom_Entitlement& OutEntitlement);

	//Fetches a single entitlement with a given Entitlement Name. The Index is used to access individual entitlements among those with the same Entitlement Name. The Index can be a value from 0 to one less than the result from EOS_Ecom_GetEntitlementsByNameCount.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyEntitlementByNameAndIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyEntitlementByNameAndIndex(FEOK_EpicAccountId LocalUserId, const FString& EntitlementName, int32 Index, FEOK_Ecom_Entitlement& OutEntitlement);

	//Fetches an item with a given ID.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyItemById")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyItemById(FEOK_EpicAccountId LocalUserId, const FString& ItemId, FEOK_Ecom_CatalogItem& OutCatalogItem);

	//Fetches an image from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyItemImageInfoByIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyItemImageInfoByIndex(FEOK_EpicAccountId LocalUserId, FEOK_Ecom_CatalogItemId ItemId, int32 ImageInfoIndex, FEOK_Ecom_KeyImageInfo& OutKeyImageInfo);

	//Fetches a release from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyItemReleaseByIndex")
	TEnumAsByte<EEOK_Result>  EOK_Ecom_CopyItemReleaseByIndex(FEOK_EpicAccountId LocalUserId, FEOK_Ecom_CatalogItemId ItemId, int32 ReleaseIndex, FEOK_Ecom_CatalogRelease& OutRelease);

	//Fetches a redeemed entitlement id from a given index. Only entitlements that were redeemed during the last EOS_Ecom_RedeemEntitlements call can be copied.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyLastRedeemedEntitlementByIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyLastRedeemedEntitlementByIndex(FEOK_EpicAccountId LocalUserId, int32 RedeemedEntitlementIndex, FString& OutRedeemedEntitlementId);

	//Fetches an offer with a given ID. The pricing and text are localized to the provided account.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyOfferById")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyOfferById(FEOK_EpicAccountId LocalUserId, const FEOK_Ecom_CatalogOfferId & OfferId, FEOK_Ecom_CatalogOffer& OutCatalogOffer);

	//Fetches an image from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyOfferImageInfoByIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyOfferImageInfoByIndex(FEOK_EpicAccountId LocalUserId, const FEOK_Ecom_CatalogOfferId & OfferId, int32 ImageInfoIndex, FEOK_Ecom_KeyImageInfo& OutKeyImageInfo);

	//Fetches an item from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyOfferItemByIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyOfferItemByIndex(FEOK_EpicAccountId LocalUserId, const FEOK_Ecom_CatalogOfferId & OfferId, int32 ItemIndex, FEOK_Ecom_CatalogItem& OutCatalogItem);

	//Fetches an offer from a given index. The pricing and text are localized to the provided account.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyOfferByIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyOfferByIndex(FEOK_EpicAccountId LocalUserId, int32 OfferIndex, FEOK_Ecom_CatalogOffer& OutCatalogOffer);

	//Fetches the transaction handle at the given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyTransactionById")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyTransactionById(FEOK_EpicAccountId LocalUserId, const FString& TransactionId, FEOK_Ecom_HTransaction& OutTransaction);

	//Fetches the transaction handle at the given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_CopyTransactionByIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_CopyTransactionByIndex(FEOK_EpicAccountId LocalUserId, int32 TransactionIndex, FEOK_Ecom_HTransaction& OutTransaction);

	//Release the memory associated with an EOS_Ecom_Entitlement structure. This must be called on data retrieved from EOS_Ecom_CopyEntitlementByIndex and EOS_Ecom_CopyEntitlementById.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_Entitlement_Release")
	void EOK_Ecom_Entitlement_Release(FEOK_Ecom_Entitlement Entitlement);

	//Fetch the number of entitlements with the given Entitlement Name that are cached for a given local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetEntitlementsByNameCount")
	int32 EOK_Ecom_GetEntitlementsByNameCount(FEOK_EpicAccountId LocalUserId, const FEOK_Ecom_EntitlementName& EntitlementName);

	//Fetch the number of entitlements that are cached for a given local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetEntitlementsCount")
	int32 EOK_Ecom_GetEntitlementsCount(FEOK_EpicAccountId LocalUserId);

	//Fetch the number of images that are associated with a given cached item for a local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetItemImageInfoCount")
	int32 EOK_Ecom_GetItemImageInfoCount(FEOK_EpicAccountId LocalUserId, FEOK_Ecom_CatalogItemId ItemId);

	//Fetch the number of releases that are associated with a given cached item for a local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetItemReleaseCount")
	int32 EOK_Ecom_GetItemReleaseCount(FEOK_EpicAccountId LocalUserId, FEOK_Ecom_CatalogItemId ItemId);

	//Fetch the number of entitlements that were redeemed during the last EOS_Ecom_RedeemEntitlements call.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetLastRedeemedEntitlementsCount")
	int32 EOK_Ecom_GetLastRedeemedEntitlementsCount(FEOK_EpicAccountId LocalUserId);

	//Fetch the number of offers that are cached for a given local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetOfferCount")
	int32 EOK_Ecom_GetOfferCount(FEOK_EpicAccountId LocalUserId);

	//Fetch the number of images that are associated with a given cached offer for a local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetOfferImageInfoCount")
	int32 EOK_Ecom_GetOfferImageInfoCount(FEOK_EpicAccountId LocalUserId, const FEOK_Ecom_CatalogOfferId & OfferId);

	//Fetch the number of items that are associated with a given cached offer for a local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetOfferItemCount")
	int32 EOK_Ecom_GetOfferItemCount(FEOK_EpicAccountId LocalUserId, const FEOK_Ecom_CatalogOfferId & OfferId);

	//Fetch the number of transactions that are cached for a given local user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_GetTransactionCount")
	int32 EOK_Ecom_GetTransactionCount(FEOK_EpicAccountId LocalUserId);

	//Release the memory associated with an EOS_Ecom_KeyImageInfo structure. This must be called on data retrieved from EOS_Ecom_CopyItemImageInfoByIndex.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_KeyImageInfo_Release")
	void EOK_Ecom_KeyImageInfo_Release(FEOK_Ecom_KeyImageInfo KeyImageInfo);

	//Fetches an entitlement from a given index.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_Transaction_CopyEntitlementByIndex")
	TEnumAsByte<EEOK_Result> EOK_Ecom_Transaction_CopyEntitlementByIndex(int32 EntitlementIndex, FEOK_Ecom_HTransaction Transaction, FEOK_Ecom_Entitlement& OutEntitlement);

	//Fetch the number of entitlements that are part of this transaction.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_Transaction_GetEntitlementsCount")
	int32 EOK_Ecom_Transaction_GetEntitlementsCount(FEOK_Ecom_HTransaction Transaction);

	//The Ecom Transaction Interface exposes getters for accessing information about a completed transaction. All Ecom Transaction Interface calls take a handle of type EOS_Ecom_HTransaction as the first parameter. An EOS_Ecom_HTransaction handle is originally returned as part of the EOS_Ecom_CheckoutCallbackInfo struct. An EOS_Ecom_HTransaction handle can also be retrieved from an EOS_HEcom handle using EOS_Ecom_CopyTransactionByIndex. It is expected that after a transaction that EOS_Ecom_Transaction_Release is called. When EOS_Platform_Release is called any remaining transactions will also be released.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Ecom Interface", DisplayName="EOS_Ecom_Transaction_GetTransactionId")
	FString EOK_Ecom_Transaction_GetTransactionId(FEOK_Ecom_HTransaction Transaction);
};
