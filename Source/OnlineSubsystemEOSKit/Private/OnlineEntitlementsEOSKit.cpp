// Copyright (C) 2024, All Rights Reserved.

#include "OnlineEntitlementsEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitEcomSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineEntitlementsEOSKit::FOnlineEntitlementsEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, EcomHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		EcomHandle = EOSKitSubsystem->EcomHandle;
	}
}

FOnlineEntitlementsEOSKit::~FOnlineEntitlementsEOSKit()
{
}

bool FOnlineEntitlementsEOSKit::QueryEntitlements(const FUniqueNetId& UserId, const FString& Namespace, const FPagedQuery& PagedQuery)
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
					UE_LOG_ONLINE(Log, TEXT("FOnlineEntitlementsEOSKit::QueryEntitlements: Delegating to UEOSKitEcomSubsystem"));
					// EcomSubsystem->QueryEntitlements(...);
					return true;
				}
			}
		}
	}
	return false;
}

EOnlineCachedResult::Type FOnlineEntitlementsEOSKit::GetCachedEntitlement(const FUniqueNetId& UserId, const FString& EntitlementId, TSharedPtr<FOnlineEntitlement>& OutEntitlement) const
{
	FScopeLock ScopeLock(&EntitlementsLock);
	const TSharedRef<FOnlineEntitlement>* Entitlement = CachedEntitlements.Find(EntitlementId);
	if (Entitlement)
	{
		OutEntitlement = *Entitlement;
		return EOnlineCachedResult::Success;
	}
	return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlineEntitlementsEOSKit::GetCachedEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement>>& OutEntitlements) const
{
	FScopeLock ScopeLock(&EntitlementsLock);
	CachedEntitlements.GenerateValueArray(OutEntitlements);
	return OutEntitlements.Num() > 0 ? EOnlineCachedResult::Success : EOnlineCachedResult::NotFound;
}

TSharedPtr<FOnlineEntitlement, ESPMode::ThreadSafe> FOnlineEntitlementsEOSKit::GetEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId)
{
	// Get entitlement - not implemented yet
	return nullptr;
}

TSharedPtr<FOnlineEntitlement, ESPMode::ThreadSafe> FOnlineEntitlementsEOSKit::GetItemEntitlement(const FUniqueNetId& UserId, const FString& ItemId)
{
	// Get item entitlement - not implemented yet
	return nullptr;
}

void FOnlineEntitlementsEOSKit::GetAllEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement, ESPMode::ThreadSafe>>& OutEntitlements)
{
	// Get all entitlements - not implemented yet
	OutEntitlements.Empty();
}

#endif // WITH_EOS_SDK

