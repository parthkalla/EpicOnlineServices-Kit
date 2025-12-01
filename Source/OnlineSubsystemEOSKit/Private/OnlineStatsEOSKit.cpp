// Copyright (C) 2024, All Rights Reserved.

#include "OnlineStatsEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitStatsSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineStatsEOSKit::FOnlineStatsEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, StatsHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		StatsHandle = EOSKitSubsystem->StatsHandle;
	}
}

FOnlineStatsEOSKit::~FOnlineStatsEOSKit()
{
}

void FOnlineStatsEOSKit::QueryStats(const FUniqueNetIdRef LocalUserId, const FUniqueNetIdRef StatsUser, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	// Delegate to EOSKitStatsSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitStatsSubsystem* StatsSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitStatsSubsystem>();
				if (StatsSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineStatsEOSKit::QueryStats: Delegating to UEOSKitStatsSubsystem"));
					// TODO: Implement QueryStats using StatsSubsystem->QueryStats(...)
					// For now, call delegate with error result
					FOnlineError ErrorResult;
					ErrorResult.bSucceeded = false;
					ErrorResult.ErrorRaw = TEXT("Not implemented");
					// In UE 5.5, FOnlineStatsQueryUserStatsComplete takes (const FOnlineError&, const TSharedPtr<const FOnlineStatsUserStats,ESPMode::ThreadSafe>&)
					TSharedPtr<const FOnlineStatsUserStats, ESPMode::ThreadSafe> EmptyStats;
					Delegate.ExecuteIfBound(ErrorResult, EmptyStats);
				}
			}
		}
	}
}

void FOnlineStatsEOSKit::QueryStats(const FUniqueNetIdRef LocalUserId, const TArray<FUniqueNetIdRef>& StatUsers, const TArray<FString>& StatNames, const FOnlineStatsQueryUsersStatsComplete& Delegate)
{
	// Delegate to EOSKitStatsSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitStatsSubsystem* StatsSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitStatsSubsystem>();
				if (StatsSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineStatsEOSKit::QueryStats (multi-user): Delegating to UEOSKitStatsSubsystem"));
					// TODO: Implement QueryStats using StatsSubsystem->QueryStats(...)
					// For now, call delegate with error result
					FOnlineError ErrorResult;
					ErrorResult.bSucceeded = false;
					ErrorResult.ErrorRaw = TEXT("Not implemented");
					// In UE 5.5, FOnlineStatsQueryUsersStatsComplete takes (const FOnlineError&, const TArray<TSharedRef<const FOnlineStatsUserStats,ESPMode::ThreadSafe>>&)
					TArray<TSharedRef<const FOnlineStatsUserStats, ESPMode::ThreadSafe>> EmptyStats;
					Delegate.ExecuteIfBound(ErrorResult, EmptyStats);
				}
			}
		}
	}
}

TSharedPtr<const FOnlineStatsUserStats> FOnlineStatsEOSKit::GetStats(const FUniqueNetIdRef StatsUserId) const
{
	// Return cached stats if available
	// TODO: Implement stats caching
	return nullptr;
}

void FOnlineStatsEOSKit::UpdateStats(const FUniqueNetIdRef LocalUserId, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats, const FOnlineStatsUpdateStatsComplete& Delegate)
{
	// Delegate to EOSKitStatsSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitStatsSubsystem* StatsSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitStatsSubsystem>();
				if (StatsSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineStatsEOSKit::UpdateStats: Delegating to UEOSKitStatsSubsystem"));
					// TODO: Implement UpdateStats using StatsSubsystem->IngestStat(...)
					// For now, call delegate with success
					FOnlineError SuccessResult;
					SuccessResult.bSucceeded = true;
					Delegate.ExecuteIfBound(SuccessResult);
				}
			}
		}
	}
}

#if !UE_BUILD_SHIPPING
void FOnlineStatsEOSKit::ResetStats(const FUniqueNetIdRef StatsUserId)
{
	// Reset stats for development/testing
	UE_LOG_ONLINE(Log, TEXT("FOnlineStatsEOSKit::ResetStats: Not implemented"));
}
#endif

#endif // WITH_EOS_SDK

