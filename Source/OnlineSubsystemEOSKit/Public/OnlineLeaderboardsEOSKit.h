// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_leaderboards.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS leaderboards
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineLeaderboardsEOSKit :
	public IOnlineLeaderboards
	, public TSharedFromThis<FOnlineLeaderboardsEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineLeaderboardsEOSKit() = delete;
	explicit FOnlineLeaderboardsEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineLeaderboardsEOSKit();

	// IOnlineLeaderboards interface
	virtual bool ReadLeaderboards(const TArray<FUniqueNetIdRef>& Players, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool ReadLeaderboardsForFriends(int32 LocalUserNum, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool ReadLeaderboardsAroundRank(int32 Rank, uint32 Range, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual void FreeStats(FOnlineLeaderboardRead& ReadObject) override;
	virtual bool WriteLeaderboards(const FName& SessionName, const FUniqueNetId& Player, FOnlineLeaderboardWrite& WriteObject) override;
	virtual bool FlushLeaderboards(const FName& SessionName) override;
	virtual bool ReadLeaderboardsAroundUser(FUniqueNetIdRef Player, uint32 Range, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool WriteOnlinePlayerRatings(const FName& SessionName, int32 LeaderboardId, const TArray<FOnlinePlayerScore>& PlayerScores) override;
	// These methods were removed from IOnlineLeaderboards in UE 5.5, but kept for backward compatibility
	bool WriteOnlinePlayerRatings(const FName& SessionName, const TArray<FOnlinePlayerScore>& PlayerScores);

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HLeaderboards LeaderboardsHandle;
	
	mutable FCriticalSection LeaderboardsLock;
};

#endif // WITH_EOS_SDK

