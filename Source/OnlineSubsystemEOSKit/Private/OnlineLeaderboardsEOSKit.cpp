// Copyright (C) 2024, All Rights Reserved.

#include "OnlineLeaderboardsEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitLeaderboardsSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineLeaderboardsEOSKit::FOnlineLeaderboardsEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, LeaderboardsHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		LeaderboardsHandle = EOSKitSubsystem->LeaderboardsHandle;
	}
}

FOnlineLeaderboardsEOSKit::~FOnlineLeaderboardsEOSKit()
{
}

bool FOnlineLeaderboardsEOSKit::ReadLeaderboards(const TArray<FUniqueNetIdRef>& Players, FOnlineLeaderboardReadRef& ReadObject)
{
	// Delegate to EOSKitLeaderboardsSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitLeaderboardsSubsystem* LeaderboardsSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitLeaderboardsSubsystem>();
				if (LeaderboardsSubsystem && Players.Num() > 0)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineLeaderboardsEOSKit::ReadLeaderboards: Delegating to UEOSKitLeaderboardsSubsystem"));
					// LeaderboardsSubsystem->QueryLeaderboardUserScores(...);
					return true;
				}
			}
		}
	}
	return false;
}

bool FOnlineLeaderboardsEOSKit::ReadLeaderboardsForFriends(int32 LocalUserNum, FOnlineLeaderboardReadRef& ReadObject)
{
	return false;
}

bool FOnlineLeaderboardsEOSKit::ReadLeaderboardsAroundRank(int32 Rank, uint32 Range, FOnlineLeaderboardReadRef& ReadObject)
{
	// Delegate to EOSKitLeaderboardsSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitLeaderboardsSubsystem* LeaderboardsSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitLeaderboardsSubsystem>();
				if (LeaderboardsSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineLeaderboardsEOSKit::ReadLeaderboardsAroundRank: Delegating to UEOSKitLeaderboardsSubsystem"));
					// LeaderboardsSubsystem->QueryLeaderboardRanks(...);
					return true;
				}
			}
		}
	}
	return false;
}

void FOnlineLeaderboardsEOSKit::FreeStats(FOnlineLeaderboardRead& ReadObject)
{
	// Cleanup
}

bool FOnlineLeaderboardsEOSKit::WriteLeaderboards(const FName& SessionName, const FUniqueNetId& Player, FOnlineLeaderboardWrite& WriteObject)
{
	return false;
}

bool FOnlineLeaderboardsEOSKit::FlushLeaderboards(const FName& SessionName)
{
	return false;
}

bool FOnlineLeaderboardsEOSKit::WriteOnlinePlayerRatings(const FName& SessionName, const TArray<FOnlinePlayerScore>& PlayerScores)
{
	return false;
}

bool FOnlineLeaderboardsEOSKit::ReadLeaderboardsAroundUser(FUniqueNetIdRef Player, uint32 Range, FOnlineLeaderboardReadRef& ReadObject)
{
	// Read leaderboards around user - not implemented yet
	return false;
}

bool FOnlineLeaderboardsEOSKit::WriteOnlinePlayerRatings(const FName& SessionName, int32 LeaderboardId, const TArray<FOnlinePlayerScore>& PlayerScores)
{
	// Write online player ratings - not implemented yet
	return false;
}

#endif // WITH_EOS_SDK

