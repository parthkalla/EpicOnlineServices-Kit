// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_friends.h"
#include "eos_friends_types.h"
THIRD_PARTY_INCLUDES_END
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOK_FriendsSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEOK_Friends_OnBlockedUsersUpdateCallback, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId, bool, bBlocked);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEOK_Friends_OnFriendsUpdateCallback, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId, const TEnumAsByte<EEOK_EFriendsStatus>&, PreviousStatus, const TEnumAsByte<EEOK_EFriendsStatus>&, CurrentStatus);
UCLASS(DisplayName="Friends Interface",meta=(DisplayName="Friends Interface"))
class OnlineSubsystemEOK_API UEOK_FriendsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FEOK_Friends_OnBlockedUsersUpdateCallback OnBlockedUserUpdate;
	static void EOS_CALL OnBlockedUserUpdateCallback(const EOS_Friends_OnBlockedUsersUpdateInfo* Data);
	/*
	 *Listen for changes to blocklist for a particular account.
	 @return A valid notification ID if successfully bound, or EOS_INVALID_NOTIFICATIONID otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_AddNotifyBlockedUsersUpdate")
	FEOK_NotificationId EOK_Friends_AddNotifyBlockedUsersUpdate(FEOK_Friends_OnBlockedUsersUpdateCallback Callback);

	FEOK_Friends_OnFriendsUpdateCallback OnFriendsUpdate;
	static void EOS_CALL OnFriendsUpdateCallback(const EOS_Friends_OnFriendsUpdateInfo* Data);
	//Listen for changes to friends for a particular account.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_AddNotifyFriendsUpdate")
	FEOK_NotificationId EOK_Friends_AddNotifyFriendsUpdate(FEOK_Friends_OnFriendsUpdateCallback Callback);


	//Retrieves the Epic Account ID of an entry from the blocklist that has already been retrieved by the EOS_Friends_QueryFriends API.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetBlockedUserAtIndex")
	FEOK_EpicAccountId EOK_Friends_GetBlockedUserAtIndex(FEOK_EpicAccountId LocalUserId, int32 Index);

	//Retrieves the number of blocked users on the blocklist that has already been retrieved by the EOS_Friends_QueryFriends API.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetBlockedUsersCount")
	int32 EOK_Friends_GetBlockedUsersCount(FEOK_EpicAccountId LocalUserId);

	//Retrieves the Epic Account ID of an entry from the friends list that has already been retrieved by the EOS_Friends_QueryFriends API. The Epic Account ID returned by this function may belong to an account that has been invited to be a friend or that has invited the local user to be a friend. To determine if the Epic Account ID returned by this function is a friend or a pending friend invitation, use the EOS_Friends_GetStatus function.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetFriendAtIndex")
	FEOK_EpicAccountId EOK_Friends_GetFriendAtIndex(FEOK_EpicAccountId LocalUserId, int32 Index);

	//Retrieves the number of friends on the friends list that has already been retrieved by the EOS_Friends_QueryFriends API.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetFriendsCount")
	int32 EOK_Friends_GetFriendsCount(FEOK_EpicAccountId LocalUserId);

	//Retrieve the friendship status between the local user and another user.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_GetStatus")
	TEnumAsByte<EEOK_EFriendsStatus> EOK_Friends_GetStatus(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId);

	//Stop listening for blocklist changes on a previously bound handler.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_RemoveNotifyBlockedUsersUpdate")
	void EOK_Friends_RemoveNotifyBlockedUsersUpdate(FEOK_NotificationId NotificationId);

	//Stop listening for friends changes on a previously bound handler.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_RemoveNotifyFriendsUpdate")
	void EOK_Friends_RemoveNotifyFriendsUpdate(FEOK_NotificationId NotificationId);

};

