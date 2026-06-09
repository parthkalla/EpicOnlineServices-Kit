// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
THIRD_PARTY_INCLUDES_START
#include "eos_friends.h"
#include "eos_friends_types.h"
THIRD_PARTY_INCLUDES_END
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Friends_RejectInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_Friends_RejectInviteCallback, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Friends_RejectInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Starts an asynchronous task that rejects a friend invitation from another user. The completion delegate is executed after the backend response has been received.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_RejectInvite")
	static UEOK_Friends_RejectInvite* EOK_Friends_RejectInvite(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Friends Interface")
	FEOK_Friends_RejectInviteCallback OnCallback;

private:
	static void EOS_CALL OnRejectInviteCallback(const EOS_Friends_RejectInviteCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_EpicAccountId Var_LocalUserId;
	FEOK_EpicAccountId Var_TargetUserId;
};
