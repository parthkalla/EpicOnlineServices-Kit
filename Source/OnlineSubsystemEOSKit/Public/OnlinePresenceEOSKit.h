// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_presence.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS presence
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlinePresenceEOSKit :
	public IOnlinePresence
	, public TSharedFromThis<FOnlinePresenceEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlinePresenceEOSKit() = delete;
	explicit FOnlinePresenceEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlinePresenceEOSKit();

	// IOnlinePresence interface
	virtual void SetPresence(const FUniqueNetId& User, const FOnlineUserPresenceStatus& Status, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate()) override;
	virtual void QueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate()) override;
	virtual EOnlineCachedResult::Type GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence, ESPMode::ThreadSafe>& OutPresence) override;
	virtual EOnlineCachedResult::Type GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence, ESPMode::ThreadSafe>& OutPresence) override;
	// These methods were removed from IOnlinePresence in UE 5.5, but kept for backward compatibility
	EOnlineCachedResult::Type GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence) const;
	EOnlineCachedResult::Type GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence>& OutPresence) const;

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HPresence PresenceHandle;
	
	mutable FCriticalSection PresenceLock;
	TMap<FUniqueNetIdRef, TSharedRef<FOnlineUserPresence>> CachedPresence;
};

#endif // WITH_EOS_SDK

