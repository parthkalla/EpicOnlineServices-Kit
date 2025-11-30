// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitFriendsTypes.generated.h"

/**
 * Friends status enumeration matching EOS_EFriendsStatus
 */
UENUM(BlueprintType)
enum class EEOSKitFriendsStatus : uint8
{
	/** The two accounts have no friendship status. */
	NotFriends = 0 UMETA(DisplayName = "Not Friends"),
	
	/** 
	 * The local account has sent a friend invite to the other account.
	 * NOTE: InviteSent is not returned by GetStatus or in FriendsUpdate callbacks unless
	 *       the local account was logged in with the EOS_AS_FriendsManagement authentication scope.
	 *       Friend invites are managed automatically by the Social Overlay.
	 */
	InviteSent = 1 UMETA(DisplayName = "Invite Sent"),
	
	/** 
	 * The other account has sent a friend invite to the local account.
	 * NOTE: InviteReceived is not returned by GetStatus or in FriendsUpdate callbacks unless
	 *       the local account was logged in with the EOS_AS_FriendsManagement authentication scope.
	 *       Friend invites are managed automatically by the Social Overlay.
	 */
	InviteReceived = 2 UMETA(DisplayName = "Invite Received"),
	
	/** The accounts have accepted friendship. */
	Friends = 3 UMETA(DisplayName = "Friends")
};

/**
 * Friend data structure containing friend information
 */
USTRUCT(BlueprintType, Category = "EOSKit|Friends")
struct EOSKITFRIENDS_API FEOSKitFriendData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Friends")
	FString DisplayName = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Friends")
	bool bIsOnline = false;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Friends")
	EEOSKitFriendsStatus InviteStatus = EEOSKitFriendsStatus::NotFriends;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Friends")
	FEOSKitEpicAccountId EpicAccountId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Friends")
	FString PresenceStatus = TEXT("");
};

