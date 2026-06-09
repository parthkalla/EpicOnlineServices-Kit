// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
THIRD_PARTY_INCLUDES_START
#include "eos_friends.h"
#include "eos_friends_types.h"
THIRD_PARTY_INCLUDES_END
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Friends_SendInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_Friends_SendInviteCallback, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Friends_SendInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Starts an asynchronous task that sends a friend invitation to another user. The completion delegate is executed after the backend response has been received. It does not indicate that the target user has responded to the friend invitation.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Friends Interface", DisplayName="EOS_Friends_SendInvite")
	static UEOK_Friends_SendInvite* EOK_Friends_SendInvite(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Friends Interface")
	FEOK_Friends_SendInviteCallback OnCallback;
private:
	static void EOS_CALL OnSendInviteCallback(const EOS_Friends_SendInviteCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_EpicAccountId Var_LocalUserId;
	FEOK_EpicAccountId Var_TargetUserId;
};
