// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitPresenceTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_presence.h"
#include "eos_presence_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// .generated.h must always be the last include
#include "EOSKitPresenceSubsystem.generated.h"

// ========================================
// Delegates
// ========================================

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOSKit_Presence_OnPresenceChangedCallback,
	const FEOSKitEpicAccountId&, LocalUserId,
	const FEOSKitEpicAccountId&, PresenceUserId);

/**
 * Presence subsystem for managing EOS Presence interface
 */
UCLASS()
class EOSKITPRESENCE_API UEOSKitPresenceSubsystem : public UGameInstanceSubsystem
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
	 * Query presence for a target user
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param TargetUserId - Epic Account ID of the target user
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Presence")
	EEOSResult QueryPresence(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId);

	/**
	 * Get presence information for a user
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param TargetUserId - Epic Account ID of the target user
	 * @param OutPresenceInfo - Output presence information
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Presence")
	bool GetPresence(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId, FEOSKitPresenceInfo& OutPresenceInfo);

	/**
	 * Add notification for presence changes
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param OnPresenceChanged - Delegate to call on presence change
	 * @return Notification ID (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Presence")
	FEOSKitNotificationId AddNotifyOnPresenceChanged(const FEOSKitEpicAccountId& LocalUserId,
		const FEOSKit_Presence_OnPresenceChangedCallback& OnPresenceChanged);

	/**
	 * Remove notification for presence changes
	 * @param NotificationId - Notification ID to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Presence")
	void RemoveNotifyOnPresenceChanged(const FEOSKitNotificationId& NotificationId);

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HPresence GetPresenceHandle() const;

	// Notification callbacks storage
	TMap<EOS_NotificationId, TSharedPtr<FCallbackBase>> NotificationCallbacks;
};

