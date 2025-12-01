// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_ui.h"
#include "eos_ui_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// .generated.h must always be the last include
#include "EOSKitUiSubsystem.generated.h"

/**
 * Delegate for display settings updated (multicast for Blueprint assignment)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSDisplaySettingsUpdated,
	bool, bIsVisible,
	bool, bIsExclusiveInput);

/**
 * Delegate for display settings updated (regular for function parameters)
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnEOSDisplaySettingsUpdatedDelegate,
	bool, bIsVisible,
	bool, bIsExclusiveInput);

/**
 * UI subsystem for managing EOS UI/Overlay interface
 */
UCLASS()
class EOSKITUI_API UEOSKitUiSubsystem : public UGameInstanceSubsystem
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
	 * Show friends overlay
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param Callback - Optional callback delegate (Note: This is async)
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UI")
	EEOSResult ShowFriends(const FEOSKitEpicAccountId& LocalUserId);

	/**
	 * Hide friends overlay
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param Callback - Optional callback delegate (Note: This is async)
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UI")
	EEOSResult HideFriends(const FEOSKitEpicAccountId& LocalUserId);

	/**
	 * Get friends overlay visibility
	 * @param LocalUserId - Epic Account ID of the local user
	 * @return True if the overlay is visible
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UI")
	bool GetFriendsVisible(const FEOSKitEpicAccountId& LocalUserId) const;

	/**
	 * Pause social overlay
	 * @param bIsPaused - True to pause, false to resume
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UI")
	EEOSResult PauseSocialOverlay(bool bIsPaused);

	/**
	 * Resume social overlay (convenience function that calls PauseSocialOverlay with false)
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UI")
	EEOSResult ResumeSocialOverlay();

	/**
	 * Get social overlay paused state
	 * @return True if the overlay is paused
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UI")
	bool IsSocialOverlayPaused() const;

	/**
	 * Add notification for display settings updated
	 * @param Callback - Callback delegate
	 * @return Notification ID (use with RemoveNotifyDisplaySettingsUpdated)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UI")
	int32 AddNotifyDisplaySettingsUpdated(const FOnEOSDisplaySettingsUpdatedDelegate& Callback);

	/**
	 * Remove notification for display settings updated
	 * @param NotificationId - Notification ID returned from AddNotifyDisplaySettingsUpdated
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|UI")
	void RemoveNotifyDisplaySettingsUpdated(int32 NotificationId);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|UI")
	FOnEOSDisplaySettingsUpdated OnDisplaySettingsUpdated;

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HUI GetUIHandle() const;

	static void EOS_CALL OnShowFriendsCallback(const EOS_UI_ShowFriendsCallbackInfo* Data);
	static void EOS_CALL OnHideFriendsCallback(const EOS_UI_HideFriendsCallbackInfo* Data);
	static void EOS_CALL OnDisplaySettingsUpdatedCallback(const EOS_UI_OnDisplaySettingsUpdatedCallbackInfo* Data);

	TMap<int32, EOS_NotificationId> DisplaySettingsNotificationIdMap;
	TMap<EOS_NotificationId, FOnEOSDisplaySettingsUpdatedDelegate> DisplaySettingsUpdatedCallbacks;
	int32 NextNotificationId;
};

