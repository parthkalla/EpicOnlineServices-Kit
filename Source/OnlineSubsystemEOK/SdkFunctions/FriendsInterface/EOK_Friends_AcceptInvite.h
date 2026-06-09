// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
THIRD_PARTY_INCLUDES_START
#include "eos_friends.h"
#include "eos_friends_types.h"
THIRD_PARTY_INCLUDES_END
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Friends_AcceptInvite.generated.h"

/**
 * Accept a friend invitation from another user.
 * @return A valid friends handle if the request was started successfully, NULL otherwise
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_Friends_AcceptInviteCallback, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Friends_AcceptInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/*
	Starts an asynchronous task that accepts a friend invitation from another user. The completion delegate is executed after the backend response has been received.
	@param LocalUserId The Epic Account ID of the local, logged-in user who is accepting the friends list invitation
	@param TargetUserId The Epic Account ID of the user who sent the friends list invitation
	 */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | Friends Interface", DisplayName="EOS_Friends_AcceptInvite")
	static UEOK_Friends_AcceptInvite* EOK_Friends_AcceptInvite(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId);

	/** Delegate to execute when the async task completes */
	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Friends Interface")
	FEOK_Friends_AcceptInviteCallback OnCallback;

private:
	static void EOS_CALL OnAcceptInviteCallback(const EOS_Friends_AcceptInviteCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_EpicAccountId Var_LocalUserId;
	FEOK_EpicAccountId Var_TargetUserId;
	
};
