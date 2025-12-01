// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_stats.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS stats
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineStatsEOSKit :
	public IOnlineStats
	, public TSharedFromThis<FOnlineStatsEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineStatsEOSKit() = delete;
	explicit FOnlineStatsEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineStatsEOSKit();

	// IOnlineStats interface
	virtual void QueryStats(const FUniqueNetIdRef LocalUserId, const FUniqueNetIdRef StatsUser, const FOnlineStatsQueryUserStatsComplete& Delegate) override;
	virtual void QueryStats(const FUniqueNetIdRef LocalUserId, const TArray<FUniqueNetIdRef>& StatUsers, const TArray<FString>& StatNames, const FOnlineStatsQueryUsersStatsComplete& Delegate) override;
	virtual TSharedPtr<const FOnlineStatsUserStats> GetStats(const FUniqueNetIdRef StatsUserId) const override;
	virtual void UpdateStats(const FUniqueNetIdRef LocalUserId, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats, const FOnlineStatsUpdateStatsComplete& Delegate) override;

#if !UE_BUILD_SHIPPING
	virtual void ResetStats(const FUniqueNetIdRef StatsUserId) override;
#endif

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HStats StatsHandle;
	
	mutable FCriticalSection StatsLock;
};

#endif // WITH_EOS_SDK

