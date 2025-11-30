// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitFriendsTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

THIRD_PARTY_INCLUDES_START
#include "eos_friends.h"
#include "eos_friends_types.h"
THIRD_PARTY_INCLUDES_END

// .generated.h must always be the last include
#include "EOSKitFriendsSubsystem.generated.h"

// ========================================
// Delegates
// ========================================

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOSKit_Friends_OnFriendsUpdateCallback, 
	const FEOSKitEpicAccountId&, LocalUserId, 
	const FEOSKitEpicAccountId&, TargetUserId, 
	EEOSKitFriendsStatus, CurrentStatus);

/**
 * Friends subsystem for managing EOS Friends interface
 */
UCLASS()
class EOSKITFRIENDS_API UEOSKitFriendsSubsystem : public UGameInstanceSubsystem
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
	 * Query friends list for a local user
	 * @param LocalUserId - Epic Account ID of the local user
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	EEOSResult QueryFriends(const FEOSKitEpicAccountId& LocalUserId);

	/**
	 * Send a friend invite
	 * @param LocalUserId - Epic Account ID of the local user sending the invite
	 * @param TargetUserId - Epic Account ID of the target user
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	EEOSResult SendInvite(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId);

	/**
	 * Accept a friend invite
	 * @param LocalUserId - Epic Account ID of the local user accepting the invite
	 * @param TargetUserId - Epic Account ID of the user who sent the invite
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	EEOSResult AcceptInvite(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId);

	/**
	 * Reject a friend invite
	 * @param LocalUserId - Epic Account ID of the local user rejecting the invite
	 * @param TargetUserId - Epic Account ID of the user who sent the invite
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	EEOSResult RejectInvite(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId);

	/**
	 * Get the number of friends for a local user
	 * @param LocalUserId - Epic Account ID of the local user
	 * @return Number of friends (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	int32 GetFriendsCount(const FEOSKitEpicAccountId& LocalUserId);

	/**
	 * Get friend at index
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param Index - Index of the friend (0-based)
	 * @param OutFriendId - Output Epic Account ID of the friend
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	bool GetFriendAtIndex(const FEOSKitEpicAccountId& LocalUserId, int32 Index, FEOSKitEpicAccountId& OutFriendId);

	/**
	 * Get friendship status between two users
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param TargetUserId - Epic Account ID of the target user
	 * @return Friends status
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	EEOSKitFriendsStatus GetStatus(const FEOSKitEpicAccountId& LocalUserId, const FEOSKitEpicAccountId& TargetUserId);

	/**
	 * Add notification for friends updates
	 * @param LocalUserId - Epic Account ID of the local user
	 * @param OnFriendsUpdate - Delegate to call on friends update
	 * @return Notification ID (0 if error)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	FEOSKitNotificationId AddNotifyFriendsUpdate(const FEOSKitEpicAccountId& LocalUserId, 
		const FEOSKit_Friends_OnFriendsUpdateCallback& OnFriendsUpdate);

	/**
	 * Remove notification for friends updates
	 * @param NotificationId - Notification ID to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friends")
	void RemoveNotifyFriendsUpdate(const FEOSKitNotificationId& NotificationId);

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HFriends GetFriendsHandle() const;

	// Notification callbacks storage
	TMap<EOS_NotificationId, TSharedPtr<FCallbackBase>> NotificationCallbacks;
};

