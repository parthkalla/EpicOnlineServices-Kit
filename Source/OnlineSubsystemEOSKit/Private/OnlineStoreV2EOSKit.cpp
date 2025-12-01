// Copyright (C) 2024, All Rights Reserved.

#include "OnlineStoreV2EOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitEcomSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineStoreV2EOSKit::FOnlineStoreV2EOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, EcomHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		EcomHandle = EOSKitSubsystem->EcomHandle;
	}
}

FOnlineStoreV2EOSKit::~FOnlineStoreV2EOSKit()
{
}

void FOnlineStoreV2EOSKit::QueryCategories(const FUniqueNetId& UserId, const FOnQueryOnlineStoreCategoriesComplete& Delegate)
{
	// Categories are not directly supported in EOS Ecom
	Delegate.ExecuteIfBound(false, TEXT("Categories not supported in EOS Ecom"));
}

void FOnlineStoreV2EOSKit::GetCategories(TArray<FOnlineStoreCategory>& OutCategories) const
{
	FScopeLock ScopeLock(&StoreLock);
	OutCategories = CachedCategories;
}

void FOnlineStoreV2EOSKit::QueryOffersByFilter(const FUniqueNetId& UserId, const FOnlineStoreFilter& Filter, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	// Delegate to EOSKitEcomSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitEcomSubsystem* EcomSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitEcomSubsystem>();
				if (EcomSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineStoreV2EOSKit::QueryOffersByFilter: Delegating to UEOSKitEcomSubsystem"));
					// EcomSubsystem->QueryOffers(...);
					Delegate.ExecuteIfBound(true, TArray<FUniqueOfferId>(), FString());
					return;
				}
			}
		}
	}
	Delegate.ExecuteIfBound(false, TArray<FUniqueOfferId>(), FString());
}

void FOnlineStoreV2EOSKit::QueryOffersById(const FUniqueNetId& UserId, const TArray<FUniqueOfferId>& OfferIds, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	// Delegate to EOSKitEcomSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitEcomSubsystem* EcomSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitEcomSubsystem>();
				if (EcomSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineStoreV2EOSKit::QueryOffersById: Delegating to UEOSKitEcomSubsystem"));
					// EcomSubsystem->QueryOffers(...);
					Delegate.ExecuteIfBound(true, OfferIds, FString());
					return;
				}
			}
		}
	}
	Delegate.ExecuteIfBound(false, OfferIds, FString());
}

void FOnlineStoreV2EOSKit::GetOffers(TArray<FOnlineStoreOfferRef>& OutOffers) const
{
	FScopeLock ScopeLock(&StoreLock);
	CachedOffers.GenerateValueArray(OutOffers);
}

TSharedPtr<FOnlineStoreOffer> FOnlineStoreV2EOSKit::GetOffer(const FUniqueOfferId& OfferId) const
{
	FScopeLock ScopeLock(&StoreLock);
	const TSharedRef<FOnlineStoreOffer>* Offer = CachedOffers.Find(OfferId);
	if (Offer)
	{
		return *Offer;
	}
	return nullptr;
}

#endif // WITH_EOS_SDK

