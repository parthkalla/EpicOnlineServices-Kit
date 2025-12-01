// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSharedTypes.h"
#include "EOSRejectFriendInviteAsync.generated.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_friends_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

/**
 * Delegate for reject friend invite completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSRejectFriendInviteComplete, const FString&, Error);

/**
 * Async node to reject a friend invite
 */
UCLASS()
class EOSKITFRIENDS_API UEOSRejectFriendInviteAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSRejectFriendInviteComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSRejectFriendInviteComplete OnFailure;

	/**
	 * Reject a friend invite from another user
	 * @param WorldContextObject - World context object
	 * @param TargetEpicAccountId - Epic Account ID of the user who sent the invite
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Reject Friend Invite",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Reject a friend invite from another user"),
		Category = "EOSKit|Friends")
	static UEOSRejectFriendInviteAsync* RejectFriendInvite(UObject* WorldContextObject, const FEOSKitEpicAccountId& TargetEpicAccountId);

	virtual void Activate() override;

private:
	static void EOS_CALL OnRejectInviteComplete(const EOS_Friends_RejectInviteCallbackInfo* Data);

	UObject* WorldContextObject;
	FEOSKitEpicAccountId TargetEpicAccountId;
};

