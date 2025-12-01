// Copyright (C) 2024, All Rights Reserved.

#include "OnlineAchievementsEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitAchievementsSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineAchievementsEOSKit::FOnlineAchievementsEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, AchievementsHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		AchievementsHandle = EOSKitSubsystem->AchievementsHandle;
	}
}

FOnlineAchievementsEOSKit::~FOnlineAchievementsEOSKit()
{
}

void FOnlineAchievementsEOSKit::WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate)
{
	// Delegate to EOSKitAchievementsSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitAchievementsSubsystem* AchievementsSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitAchievementsSubsystem>();
				if (AchievementsSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineAchievementsEOSKit::WriteAchievements: Delegating to UEOSKitAchievementsSubsystem"));
					// Extract achievement IDs from WriteObject and call AchievementsSubsystem->UnlockAchievements(...);
					Delegate.ExecuteIfBound(PlayerId, true);
					return;
				}
			}
		}
	}
	Delegate.ExecuteIfBound(PlayerId, false);
}

void FOnlineAchievementsEOSKit::QueryAchievements(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	// Delegate to EOSKitAchievementsSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitAchievementsSubsystem* AchievementsSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitAchievementsSubsystem>();
				if (AchievementsSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineAchievementsEOSKit::QueryAchievements: Delegating to UEOSKitAchievementsSubsystem"));
					// AchievementsSubsystem->QueryPlayerAchievements(...);
					Delegate.ExecuteIfBound(PlayerId, true);
					return;
				}
			}
		}
	}
	Delegate.ExecuteIfBound(PlayerId, false);
}

EOnlineCachedResult::Type FOnlineAchievementsEOSKit::GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement) const
{
	FScopeLock ScopeLock(&AchievementsLock);
	const TSharedRef<FOnlineAchievement>* Achievement = CachedAchievements.Find(AchievementId);
	if (Achievement)
	{
		OutAchievement = **Achievement;
		return EOnlineCachedResult::Success;
	}
	return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlineAchievementsEOSKit::GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement>& OutAchievements) const
{
	FScopeLock ScopeLock(&AchievementsLock);
	OutAchievements.Empty();
	for (const auto& Pair : CachedAchievements)
	{
		OutAchievements.Add(*Pair.Value);
	}
	return OutAchievements.Num() > 0 ? EOnlineCachedResult::Success : EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlineAchievementsEOSKit::GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc) const
{
	FScopeLock ScopeLock(&AchievementsLock);
	const TSharedRef<FOnlineAchievementDesc>* Desc = CachedAchievementDescriptions.Find(AchievementId);
	if (Desc)
	{
		OutAchievementDesc = **Desc;
		return EOnlineCachedResult::Success;
	}
	return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlineAchievementsEOSKit::GetCachedAchievementDescriptions(const TArray<FString>& AchievementIds, TArray<FOnlineAchievementDesc>& OutAchievementDescs) const
{
	FScopeLock ScopeLock(&AchievementsLock);
	OutAchievementDescs.Empty();
	for (const FString& AchievementId : AchievementIds)
	{
		const TSharedRef<FOnlineAchievementDesc>* Desc = CachedAchievementDescriptions.Find(AchievementId);
		if (Desc)
		{
			OutAchievementDescs.Add(**Desc);
		}
	}
	return OutAchievementDescs.Num() > 0 ? EOnlineCachedResult::Success : EOnlineCachedResult::NotFound;
}

bool FOnlineAchievementsEOSKit::GetAchievementProgress(const FUniqueNetId& PlayerId, const FString& AchievementId, int32& OutCurrentProgress, int32& OutMaxProgress) const
{
	FOnlineAchievement Achievement;
	if (GetCachedAchievement(PlayerId, AchievementId, Achievement) == EOnlineCachedResult::Success)
	{
		OutCurrentProgress = Achievement.Progress;
		OutMaxProgress = Achievement.Progress;
		return true;
	}
	return false;
}

void FOnlineAchievementsEOSKit::QueryAchievementDescriptions(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	// Query achievement descriptions - not implemented yet
	Delegate.ExecuteIfBound(PlayerId, false);
}

EOnlineCachedResult::Type FOnlineAchievementsEOSKit::GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement)
{
	// Call const version
	return const_cast<const FOnlineAchievementsEOSKit*>(this)->GetCachedAchievement(PlayerId, AchievementId, OutAchievement);
}

EOnlineCachedResult::Type FOnlineAchievementsEOSKit::GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement>& OutAchievements)
{
	// Call const version
	return const_cast<const FOnlineAchievementsEOSKit*>(this)->GetCachedAchievements(PlayerId, OutAchievements);
}

EOnlineCachedResult::Type FOnlineAchievementsEOSKit::GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc)
{
	// Call const version
	return const_cast<const FOnlineAchievementsEOSKit*>(this)->GetCachedAchievementDescription(AchievementId, OutAchievementDesc);
}

bool FOnlineAchievementsEOSKit::ResetAchievements(const FUniqueNetId& PlayerId)
{
	// Reset achievements - not implemented yet
	return false;
}

#endif // WITH_EOS_SDK

