// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSharedTypes.h"
#include "EOSAcceptFriendInviteAsync.generated.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_friends_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

/**
 * Delegate for accept friend invite completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSAcceptFriendInviteComplete, const FString&, Error);

/**
 * Async node to accept a friend invite
 */
UCLASS()
class EOSKITFRIENDS_API UEOSAcceptFriendInviteAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSAcceptFriendInviteComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSAcceptFriendInviteComplete OnFailure;

	/**
	 * Accept a friend invite from another user
	 * @param WorldContextObject - World context object
	 * @param TargetEpicAccountId - Epic Account ID of the user who sent the invite
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Accept Friend Invite",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Accept a friend invite from another user"),
		Category = "EOSKit|Friends")
	static UEOSAcceptFriendInviteAsync* AcceptFriendInvite(UObject* WorldContextObject, const FEOSKitEpicAccountId& TargetEpicAccountId);

	virtual void Activate() override;

private:
	static void EOS_CALL OnAcceptInviteComplete(const EOS_Friends_AcceptInviteCallbackInfo* Data);

	UObject* WorldContextObject;
	FEOSKitEpicAccountId TargetEpicAccountId;
};

