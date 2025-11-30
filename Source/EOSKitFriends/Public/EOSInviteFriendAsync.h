// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSharedTypes.h"
#include "EOSInviteFriendAsync.generated.h"

/**
 * Delegate for friend invite completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSInviteFriendComplete, const FString&, Error);

/**
 * Async node to send a friend invite
 */
UCLASS()
class EOSKITFRIENDS_API UEOSInviteFriendAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSInviteFriendComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSInviteFriendComplete OnFailure;

	/**
	 * Send a friend invite to a target user
	 * @param WorldContextObject - World context object
	 * @param TargetEpicAccountId - Epic Account ID of the target user
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Invite Friend",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Send a friend invite to another user"),
		Category = "EOSKit|Friends")
	static UEOSInviteFriendAsync* InviteFriend(UObject* WorldContextObject, const FEOSKitEpicAccountId& TargetEpicAccountId);

	virtual void Activate() override;

private:
	static void EOS_CALL OnSendInviteComplete(const EOS_Friends_SendInviteCallbackInfo* Data);

	UObject* WorldContextObject;
	FEOSKitEpicAccountId TargetEpicAccountId;
};

