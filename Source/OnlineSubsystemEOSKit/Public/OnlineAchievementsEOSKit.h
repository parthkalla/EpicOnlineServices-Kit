// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK
	#include "eos_achievements.h"
#endif

class FOnlineSubsystemEOSKit;

#if WITH_EOS_SDK

/**
 * Interface for EOS achievements
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineAchievementsEOSKit :
	public IOnlineAchievements
	, public TSharedFromThis<FOnlineAchievementsEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineAchievementsEOSKit() = delete;
	explicit FOnlineAchievementsEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineAchievementsEOSKit();

	// IOnlineAchievements interface
	virtual void WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate = FOnAchievementsWrittenDelegate()) override;
	virtual void QueryAchievements(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate = FOnQueryAchievementsCompleteDelegate()) override;
	virtual void QueryAchievementDescriptions(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate) override;
	virtual EOnlineCachedResult::Type GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement) override;
	virtual EOnlineCachedResult::Type GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement>& OutAchievements) override;
	virtual EOnlineCachedResult::Type GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc) override;
	virtual bool ResetAchievements(const FUniqueNetId& PlayerId) override;
	// These methods were removed from IOnlineAchievements in UE 5.5, but kept for backward compatibility
	virtual EOnlineCachedResult::Type GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement) const;
	virtual EOnlineCachedResult::Type GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement>& OutAchievements) const;
	virtual EOnlineCachedResult::Type GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc) const;
	virtual EOnlineCachedResult::Type GetCachedAchievementDescriptions(const TArray<FString>& AchievementIds, TArray<FOnlineAchievementDesc>& OutAchievementDescs) const;
	virtual bool GetAchievementProgress(const FUniqueNetId& PlayerId, const FString& AchievementId, int32& OutCurrentProgress, int32& OutMaxProgress) const;

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
	EOS_HAchievements AchievementsHandle;
	
	mutable FCriticalSection AchievementsLock;
	TMap<FString, TSharedRef<FOnlineAchievement>> CachedAchievements;
	TMap<FString, TSharedRef<FOnlineAchievementDesc>> CachedAchievementDescriptions;
};

#endif // WITH_EOS_SDK

