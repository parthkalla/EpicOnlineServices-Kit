// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitEcomSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_ecom.h"
#include "eos_ecom_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"

void UEOSKitEcomSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("EOSKitEcomSubsystem: Initialized"));
}

void UEOSKitEcomSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitEcomSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HEcom UEOSKitEcomSubsystem::GetEcomHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetEcomInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitEcomSubsystem::QueryOffers(const FEOSKitEpicAccountId& LocalUserId, const FString& OverrideCatalogNamespace)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Ecom_QueryOffersOptions Options = {};
	Options.ApiVersion = EOS_ECOM_QUERYOFFERS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.OverrideCatalogNamespace = OverrideCatalogNamespace.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*OverrideCatalogNamespace);

	// Note: This is a synchronous wrapper, but QueryOffers is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitEcomSubsystem::QueryOwnership(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& CatalogItemIds, const FString& OverrideCatalogNamespace)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	if (CatalogItemIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: CatalogItemIds array is empty"));
		return EEOSResult::EOS_InvalidParameters;
	}

	EOS_Ecom_QueryOwnershipOptions Options = {};
	Options.ApiVersion = EOS_ECOM_QUERYOWNERSHIP_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.CatalogNamespace = OverrideCatalogNamespace.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*OverrideCatalogNamespace);

	// Convert FString array to const char* array
	TArray<const char*> CatalogItemIdPtrs;
	CatalogItemIdPtrs.Reserve(CatalogItemIds.Num());
	
	// Use stack-allocated converters to avoid move constructor issues
	TArray<FTCHARToUTF8, TInlineAllocator<32>> Converters;
	Converters.Reserve(CatalogItemIds.Num());

	for (const FString& CatalogItemId : CatalogItemIds)
	{
		Converters.Emplace(*CatalogItemId);
		CatalogItemIdPtrs.Add(Converters.Last().Get());
	}

	Options.CatalogItemIds = CatalogItemIdPtrs.GetData();
	Options.CatalogItemIdCount = CatalogItemIdPtrs.Num();

	// Note: This is a synchronous wrapper, but QueryOwnership is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitEcomSubsystem::QueryOwnershipToken(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& CatalogItemIds, const FString& OverrideCatalogNamespace)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	if (CatalogItemIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: CatalogItemIds array is empty"));
		return EEOSResult::EOS_InvalidParameters;
	}

	EOS_Ecom_QueryOwnershipTokenOptions Options = {};
	Options.ApiVersion = EOS_ECOM_QUERYOWNERSHIPTOKEN_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.CatalogNamespace = OverrideCatalogNamespace.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*OverrideCatalogNamespace);

	// Convert FString array to const char* array
	TArray<const char*> CatalogItemIdPtrs;
	CatalogItemIdPtrs.Reserve(CatalogItemIds.Num());
	
	// Use stack-allocated converters to avoid move constructor issues
	TArray<FTCHARToUTF8, TInlineAllocator<32>> Converters;
	Converters.Reserve(CatalogItemIds.Num());

	for (const FString& CatalogItemId : CatalogItemIds)
	{
		Converters.Emplace(*CatalogItemId);
		CatalogItemIdPtrs.Add(Converters.Last().Get());
	}

	Options.CatalogItemIds = CatalogItemIdPtrs.GetData();
	Options.CatalogItemIdCount = CatalogItemIdPtrs.Num();

	// Note: This is a synchronous wrapper, but QueryOwnershipToken is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitEcomSubsystem::QueryEntitlements(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& EntitlementNames, bool bIncludeRedeemed, const FString& OverrideCatalogNamespace)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Ecom_QueryEntitlementsOptions Options = {};
	Options.ApiVersion = EOS_ECOM_QUERYENTITLEMENTS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.bIncludeRedeemed = bIncludeRedeemed ? EOS_TRUE : EOS_FALSE;
	// OverrideCatalogNamespace was removed in SDK 1.18

	// Convert FString array to const char* array if provided
	if (EntitlementNames.Num() > 0)
	{
		TArray<const char*> EntitlementNamePtrs;
		EntitlementNamePtrs.Reserve(EntitlementNames.Num());
		TArray<FTCHARToUTF8, TInlineAllocator<32>> Converters;
		Converters.Reserve(EntitlementNames.Num());

		for (const FString& EntitlementName : EntitlementNames)
		{
			Converters.Emplace(*EntitlementName);
			EntitlementNamePtrs.Add(Converters.Last().Get());
		}

		Options.EntitlementNames = EntitlementNamePtrs.GetData();
		Options.EntitlementNameCount = EntitlementNamePtrs.Num();
	}
	else
	{
		Options.EntitlementNames = nullptr;
		Options.EntitlementNameCount = 0;
	}

	// Note: This is a synchronous wrapper, but QueryEntitlements is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitEcomSubsystem::RedeemEntitlements(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& EntitlementIds)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	if (EntitlementIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: EntitlementIds array is empty"));
		return EEOSResult::EOS_InvalidParameters;
	}

	EOS_Ecom_RedeemEntitlementsOptions Options = {};
	Options.ApiVersion = EOS_ECOM_REDEEMENTITLEMENTS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// Convert FString array to const char* array
	TArray<const char*> EntitlementIdPtrs;
	EntitlementIdPtrs.Reserve(EntitlementIds.Num());
	TArray<FTCHARToUTF8, TInlineAllocator<32>> Converters;
	Converters.Reserve(EntitlementIds.Num());

	for (const FString& EntitlementId : EntitlementIds)
	{
		Converters.Emplace(*EntitlementId);
		EntitlementIdPtrs.Add(Converters.Last().Get());
	}

	Options.EntitlementIds = EntitlementIdPtrs.GetData();
	Options.EntitlementIdCount = EntitlementIdPtrs.Num();

	// Note: This is a synchronous wrapper, but RedeemEntitlements is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitEcomSubsystem::Checkout(const FEOSKitEpicAccountId& LocalUserId, const TArray<FString>& OfferIds)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	if (OfferIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: OfferIds array is empty"));
		return EEOSResult::EOS_InvalidParameters;
	}

	EOS_Ecom_CheckoutOptions Options = {};
	Options.ApiVersion = EOS_ECOM_CHECKOUT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// Convert FString array to EOS_Ecom_CheckoutEntry array
	TArray<EOS_Ecom_CheckoutEntry> Entries;
	Entries.Reserve(OfferIds.Num());
	TArray<FTCHARToUTF8, TInlineAllocator<32>> Converters;
	Converters.Reserve(OfferIds.Num());

	for (const FString& OfferId : OfferIds)
	{
		EOS_Ecom_CheckoutEntry Entry = {};
		Entry.ApiVersion = EOS_ECOM_CHECKOUTENTRY_API_LATEST;
		Converters.Emplace(*OfferId);
		Entry.OfferId = Converters.Last().Get();
		Entries.Add(Entry);
	}

	Options.Entries = Entries.GetData();
	Options.EntryCount = Entries.Num();

	// Note: This is a synchronous wrapper, but Checkout is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

int32 UEOSKitEcomSubsystem::GetOfferCount(const FEOSKitEpicAccountId& LocalUserId)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return 0;
	}

	EOS_Ecom_GetOfferCountOptions Options = {};
	Options.ApiVersion = EOS_ECOM_GETOFFERCOUNT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	return static_cast<int32>(EOS_Ecom_GetOfferCount(EcomHandle, &Options));
}

bool UEOSKitEcomSubsystem::CopyOfferByIndex(const FEOSKitEpicAccountId& LocalUserId, int32 Index, FEOSKitCatalogOffer& OutOffer)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return false;
	}

	EOS_Ecom_CopyOfferByIndexOptions Options = {};
	Options.ApiVersion = EOS_ECOM_COPYOFFERBYINDEX_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.OfferIndex = Index;

	EOS_Ecom_CatalogOffer* Offer = nullptr;
	EOS_EResult Result = EOS_Ecom_CopyOfferByIndex(EcomHandle, &Options, &Offer);

	if (Result == EOS_EResult::EOS_Success && Offer)
	{
		OutOffer.OfferId = Offer->Id ? UTF8_TO_TCHAR(Offer->Id) : TEXT("");
		OutOffer.CatalogNamespace = Offer->CatalogNamespace ? UTF8_TO_TCHAR(Offer->CatalogNamespace) : TEXT("");
		OutOffer.TitleText = Offer->TitleText ? UTF8_TO_TCHAR(Offer->TitleText) : TEXT("");
		OutOffer.DescriptionText = Offer->DescriptionText ? UTF8_TO_TCHAR(Offer->DescriptionText) : TEXT("");
		OutOffer.LongDescriptionText = Offer->LongDescriptionText ? UTF8_TO_TCHAR(Offer->LongDescriptionText) : TEXT("");
		OutOffer.CurrencyCode = Offer->CurrencyCode ? UTF8_TO_TCHAR(Offer->CurrencyCode) : TEXT("");
		OutOffer.OriginalPrice64 = Offer->OriginalPrice64;
		OutOffer.CurrentPrice64 = Offer->CurrentPrice64;
		OutOffer.DecimalPoint = Offer->DecimalPoint;
		OutOffer.DiscountPercentage = Offer->DiscountPercentage;
		OutOffer.ExpirationTimestamp = Offer->ExpirationTimestamp;
		OutOffer.PurchaseLimit = Offer->PurchaseLimit;
		OutOffer.bAvailableForPurchase = Offer->bAvailableForPurchase == EOS_TRUE;
		OutOffer.ReleaseDateTimestamp = Offer->ReleaseDateTimestamp;
		OutOffer.EffectiveDateTimestamp = Offer->EffectiveDateTimestamp;

		EOS_Ecom_CatalogOffer_Release(Offer);
		return true;
	}

	return false;
}

bool UEOSKitEcomSubsystem::CopyOfferById(const FEOSKitEpicAccountId& LocalUserId, const FString& OfferId, FEOSKitCatalogOffer& OutOffer)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return false;
	}

	EOS_Ecom_CopyOfferByIdOptions Options = {};
	Options.ApiVersion = EOS_ECOM_COPYOFFERBYID_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.OfferId = TCHAR_TO_UTF8(*OfferId);

	EOS_Ecom_CatalogOffer* Offer = nullptr;
	EOS_EResult Result = EOS_Ecom_CopyOfferById(EcomHandle, &Options, &Offer);

	if (Result == EOS_EResult::EOS_Success && Offer)
	{
		OutOffer.OfferId = Offer->Id ? UTF8_TO_TCHAR(Offer->Id) : TEXT("");
		OutOffer.CatalogNamespace = Offer->CatalogNamespace ? UTF8_TO_TCHAR(Offer->CatalogNamespace) : TEXT("");
		OutOffer.TitleText = Offer->TitleText ? UTF8_TO_TCHAR(Offer->TitleText) : TEXT("");
		OutOffer.DescriptionText = Offer->DescriptionText ? UTF8_TO_TCHAR(Offer->DescriptionText) : TEXT("");
		OutOffer.LongDescriptionText = Offer->LongDescriptionText ? UTF8_TO_TCHAR(Offer->LongDescriptionText) : TEXT("");
		OutOffer.CurrencyCode = Offer->CurrencyCode ? UTF8_TO_TCHAR(Offer->CurrencyCode) : TEXT("");
		OutOffer.OriginalPrice64 = Offer->OriginalPrice64;
		OutOffer.CurrentPrice64 = Offer->CurrentPrice64;
		OutOffer.DecimalPoint = Offer->DecimalPoint;
		OutOffer.DiscountPercentage = Offer->DiscountPercentage;
		OutOffer.ExpirationTimestamp = Offer->ExpirationTimestamp;
		OutOffer.PurchaseLimit = Offer->PurchaseLimit;
		OutOffer.bAvailableForPurchase = Offer->bAvailableForPurchase == EOS_TRUE;
		OutOffer.ReleaseDateTimestamp = Offer->ReleaseDateTimestamp;
		OutOffer.EffectiveDateTimestamp = Offer->EffectiveDateTimestamp;

		EOS_Ecom_CatalogOffer_Release(Offer);
		return true;
	}

	return false;
}

int32 UEOSKitEcomSubsystem::GetEntitlementsCount(const FEOSKitEpicAccountId& LocalUserId)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return 0;
	}

	EOS_Ecom_GetEntitlementsCountOptions Options = {};
	Options.ApiVersion = EOS_ECOM_GETENTITLEMENTSCOUNT_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	return static_cast<int32>(EOS_Ecom_GetEntitlementsCount(EcomHandle, &Options));
}

bool UEOSKitEcomSubsystem::CopyEntitlementByIndex(const FEOSKitEpicAccountId& LocalUserId, int32 Index, FEOSKitEntitlement& OutEntitlement)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return false;
	}

	EOS_Ecom_CopyEntitlementByIndexOptions Options = {};
	Options.ApiVersion = EOS_ECOM_COPYENTITLEMENTBYINDEX_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.EntitlementIndex = Index;

	EOS_Ecom_Entitlement* Entitlement = nullptr;
	EOS_EResult Result = EOS_Ecom_CopyEntitlementByIndex(EcomHandle, &Options, &Entitlement);

	if ((Result == EOS_EResult::EOS_Success || Result == EOS_EResult::EOS_Ecom_EntitlementStale) && Entitlement)
	{
		OutEntitlement.EntitlementName = Entitlement->EntitlementName ? UTF8_TO_TCHAR(Entitlement->EntitlementName) : TEXT("");
		OutEntitlement.EntitlementId = Entitlement->EntitlementId ? UTF8_TO_TCHAR(Entitlement->EntitlementId) : TEXT("");
		OutEntitlement.CatalogItemId = Entitlement->CatalogItemId ? UTF8_TO_TCHAR(Entitlement->CatalogItemId) : TEXT("");
		OutEntitlement.bRedeemed = Entitlement->bRedeemed == EOS_TRUE;
		OutEntitlement.EndTimestamp = Entitlement->EndTimestamp;

		EOS_Ecom_Entitlement_Release(Entitlement);
		return true;
	}

	return false;
}

bool UEOSKitEcomSubsystem::CopyEntitlementById(const FEOSKitEpicAccountId& LocalUserId, const FString& EntitlementId, FEOSKitEntitlement& OutEntitlement)
{
	EOS_HEcom EcomHandle = GetEcomHandle();
	if (!EcomHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitEcom: Failed to get Ecom Handle"));
		return false;
	}

	EOS_Ecom_CopyEntitlementByIdOptions Options = {};
	Options.ApiVersion = EOS_ECOM_COPYENTITLEMENTBYID_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.EntitlementId = TCHAR_TO_UTF8(*EntitlementId);

	EOS_Ecom_Entitlement* Entitlement = nullptr;
	EOS_EResult Result = EOS_Ecom_CopyEntitlementById(EcomHandle, &Options, &Entitlement);

	if ((Result == EOS_EResult::EOS_Success || Result == EOS_EResult::EOS_Ecom_EntitlementStale) && Entitlement)
	{
		OutEntitlement.EntitlementName = Entitlement->EntitlementName ? UTF8_TO_TCHAR(Entitlement->EntitlementName) : TEXT("");
		OutEntitlement.EntitlementId = Entitlement->EntitlementId ? UTF8_TO_TCHAR(Entitlement->EntitlementId) : TEXT("");
		OutEntitlement.CatalogItemId = Entitlement->CatalogItemId ? UTF8_TO_TCHAR(Entitlement->CatalogItemId) : TEXT("");
		OutEntitlement.bRedeemed = Entitlement->bRedeemed == EOS_TRUE;
		OutEntitlement.EndTimestamp = Entitlement->EndTimestamp;

		EOS_Ecom_Entitlement_Release(Entitlement);
		return true;
	}

	return false;
}

