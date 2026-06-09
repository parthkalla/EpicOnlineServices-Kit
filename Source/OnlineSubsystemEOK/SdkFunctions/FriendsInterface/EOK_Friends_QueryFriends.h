// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
THIRD_PARTY_INCLUDES_START
#include "eos_friends.h"
#include "eos_friends_types.h"
THIRD_PARTY_INCLUDES_END
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Friends_QueryFriends.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Friends_QueryFriendsCallback, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_EpicAccountId&, LocalUserId);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Friends_QueryFriends : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/*
	 *Starts an asynchronous task that reads the user's friends list and blocklist from the backend service, caching it for future use.

	When the Social Overlay is enabled then this will be called automatically. The Social Overlay is enabled by default (see EOS_PF_DISABLE_SOCIAL_OVERLAY).
	 */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_QueryFriends")
	static UEOK_Friends_QueryFriends* EOK_Friends_QueryFriends(FEOK_EpicAccountId LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Friends Interface")
	FEOK_Friends_QueryFriendsCallback OnCallback;

private:
	static void EOS_CALL OnQueryFriendsCallback(const EOS_Friends_QueryFriendsCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_EpicAccountId Var_LocalUserId;
};
