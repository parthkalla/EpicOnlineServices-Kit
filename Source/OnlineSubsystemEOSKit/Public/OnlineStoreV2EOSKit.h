// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#if __has_include("Interfaces/OnlineStoreInterfaceV2.h")
#include "Interfaces/OnlineStoreInterfaceV2.h"
#elif __has_include("Interfaces/OnlineStoreV2Interface.h")
#include "Interfaces/OnlineStoreV2Interface.h"
#else
// Forward declare if header not available
class IOnlineStoreV2;
#endif
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_ecom.h"
#endif

class FOnlineSubsystemEOSKit;

// Compatibility aliases for older plugin code
#if defined(FUniqueOfferId)
using FUniqueStoreOfferId = FUniqueOfferId;
#else
typedef FString FUniqueStoreOfferId;
#endif

#if defined(FOnQueryOnlineStoreCategoriesComplete)
using FOnQueryOnlineStoreCategoriesCompleteDelegate = FOnQueryOnlineStoreCategoriesComplete;
#endif

#if defined(FOnQueryOnlineStoreOffersComplete)
using FOnQueryOnlineStoreOffersCompleteDelegate = FOnQueryOnlineStoreOffersComplete;
#endif

#if WITH_EOS_SDK

/**
 * Interface for EOS store (part of Ecom)
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineStoreV2EOSKit :
	public IOnlineStoreV2
	, public TSharedFromThis<FOnlineStoreV2EOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineStoreV2EOSKit() = delete;
	explicit FOnlineStoreV2EOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineStoreV2EOSKit();

	// IOnlineStoreV2 interface
	virtual void QueryCategories(const FUniqueNetId& UserId, const FOnQueryOnlineStoreCategoriesComplete& Delegate = FOnQueryOnlineStoreCategoriesComplete()) override;
	virtual void GetCategories(TArray<FOnlineStoreCategory>& OutCategories) const override;
	virtual void QueryOffersByFilter(const FUniqueNetId& UserId, const FOnlineStoreFilter& Filter, const FOnQueryOnlineStoreOffersComplete& Delegate = FOnQueryOnlineStoreOffersComplete()) override;
	virtual void QueryOffersById(const FUniqueNetId& UserId, const TArray<FUniqueOfferId>& OfferIds, const FOnQueryOnlineStoreOffersComplete& Delegate = FOnQueryOnlineStoreOffersComplete()) override;
	virtual void GetOffers(TArray<FOnlineStoreOfferRef>& OutOffers) const override;
	virtual TSharedPtr<FOnlineStoreOffer> GetOffer(const FUniqueOfferId& OfferId) const override;

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HEcom EcomHandle;
	
	mutable FCriticalSection StoreLock;
	TArray<FOnlineStoreCategory> CachedCategories;
	TMap<FUniqueOfferId, TSharedRef<FOnlineStoreOffer>> CachedOffers;
};

#endif // WITH_EOS_SDK

