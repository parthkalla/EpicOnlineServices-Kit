// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitAchievementsTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_achievements.h"
#include "eos_achievements_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// .generated.h must always be the last include
#include "EOSKitAchievementsSubsystem.generated.h"

/**
 * Delegate for achievements unlocked notification (multicast for Blueprint assignment)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSAchievementsUnlocked, 
	const FEOSKitProductUserId&, UserId, 
	const FString&, AchievementId, 
	int64, UnlockTime);

/**
 * Delegate for achievements unlocked notification (regular for function parameters)
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnEOSAchievementsUnlockedDelegate, 
	const FEOSKitProductUserId&, UserId, 
	const FString&, AchievementId, 
	int64, UnlockTime);

/**
 * Achievements subsystem for managing EOS Achievements interface
 */
UCLASS()
class EOSKITSTATS_API UEOSKitAchievementsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// SDK Function Wrappers
	// ========================================

	/**
	 * Query achievement definitions
	 * @param LocalUserId - Product User ID of the local user
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	EEOSResult QueryDefinitions(const FEOSKitProductUserId& LocalUserId);

	/**
	 * Query player achievements
	 * @param LocalUserId - Product User ID of the local user
	 * @param TargetUserId - Product User ID of the target user
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	EEOSResult QueryPlayerAchievements(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& TargetUserId);

	/**
	 * Unlock achievements
	 * @param LocalUserId - Product User ID of the local user
	 * @param AchievementIds - Array of achievement IDs to unlock
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	EEOSResult UnlockAchievements(const FEOSKitProductUserId& LocalUserId, const TArray<FString>& AchievementIds);

	/**
	 * Get achievement definition count
	 * @return Number of achievement definitions (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	int32 GetAchievementDefinitionCount();

	/**
	 * Copy achievement definition by index
	 * @param Index - Index of the achievement definition
	 * @param OutDefinition - Output achievement definition
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	bool CopyAchievementDefinitionV2ByIndex(int32 Index, FEOSAchievementDefinition& OutDefinition);

	/**
	 * Copy achievement definition by achievement ID
	 * @param AchievementId - Achievement ID
	 * @param OutDefinition - Output achievement definition
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	bool CopyAchievementDefinitionV2ByAchievementId(const FString& AchievementId, FEOSAchievementDefinition& OutDefinition);

	/**
	 * Get player achievement count
	 * @param LocalUserId - Product User ID of the local user
	 * @param TargetUserId - Product User ID of the target user
	 * @return Number of player achievements (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	int32 GetPlayerAchievementCount(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& TargetUserId);

	/**
	 * Copy player achievement by index
	 * @param LocalUserId - Product User ID of the local user
	 * @param TargetUserId - Product User ID of the target user
	 * @param Index - Index of the player achievement
	 * @param OutAchievement - Output player achievement
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	bool CopyPlayerAchievementByIndex(const FEOSKitProductUserId& LocalUserId, 
		const FEOSKitProductUserId& TargetUserId, 
		int32 Index, 
		FEOSPlayerAchievement& OutAchievement);

	/**
	 * Copy player achievement by achievement ID
	 * @param LocalUserId - Product User ID of the local user
	 * @param TargetUserId - Product User ID of the target user
	 * @param AchievementId - Achievement ID
	 * @param OutAchievement - Output player achievement
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	bool CopyPlayerAchievementByAchievementId(const FEOSKitProductUserId& LocalUserId, 
		const FEOSKitProductUserId& TargetUserId, 
		const FString& AchievementId, 
		FEOSPlayerAchievement& OutAchievement);

	/**
	 * Add notification for achievements unlocked
	 * @param Callback - Callback delegate
	 * @return Notification ID (use with RemoveNotifyAchievementsUnlocked)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	int32 AddNotifyAchievementsUnlockedV2(const FOnEOSAchievementsUnlockedDelegate& Callback);

	/**
	 * Remove notification for achievements unlocked
	 * @param NotificationId - Notification ID returned from AddNotifyAchievementsUnlockedV2
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Achievements")
	void RemoveNotifyAchievementsUnlockedV2(int32 NotificationId);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Achievements")
	FOnEOSAchievementsUnlocked OnAchievementsUnlocked;

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HAchievements GetAchievementsHandle() const;

	static void EOS_CALL OnAchievementsUnlockedV2Callback(const EOS_Achievements_OnAchievementsUnlockedCallbackV2Info* Data);

	TMap<int32, EOS_NotificationId> NotificationIdMap;
	TMap<EOS_NotificationId, FOnEOSAchievementsUnlockedDelegate> AchievementsUnlockedCallbacks;
	int32 NextNotificationId;
};

