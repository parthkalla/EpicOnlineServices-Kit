// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_ecom.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS entitlements (part of Ecom)
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineEntitlementsEOSKit :
	public IOnlineEntitlements
	, public TSharedFromThis<FOnlineEntitlementsEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineEntitlementsEOSKit() = delete;
	explicit FOnlineEntitlementsEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineEntitlementsEOSKit();

	// IOnlineEntitlements interface
	virtual bool QueryEntitlements(const FUniqueNetId& UserId, const FString& Namespace, const FPagedQuery& PagedQuery) override;
	virtual TSharedPtr<FOnlineEntitlement, ESPMode::ThreadSafe> GetEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId) override;
	virtual TSharedPtr<FOnlineEntitlement, ESPMode::ThreadSafe> GetItemEntitlement(const FUniqueNetId& UserId, const FString& ItemId) override;
	virtual void GetAllEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement, ESPMode::ThreadSafe>>& OutEntitlements) override;
	// Note: GetCachedEntitlement and GetCachedEntitlements are not in UE 5.5 interface
	EOnlineCachedResult::Type GetCachedEntitlement(const FUniqueNetId& UserId, const FString& EntitlementId, TSharedPtr<FOnlineEntitlement>& OutEntitlement) const;
	EOnlineCachedResult::Type GetCachedEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement>>& OutEntitlements) const;

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HEcom EcomHandle;
	
	mutable FCriticalSection EntitlementsLock;
	TMap<FString, TSharedRef<FOnlineEntitlement>> CachedEntitlements;
};

#endif // WITH_EOS_SDK

