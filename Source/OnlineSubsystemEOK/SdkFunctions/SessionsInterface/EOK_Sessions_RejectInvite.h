// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sessions.h"
#include "eos_sessions_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_Sessions_RejectInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnRejectInviteCallback, const TEnumAsByte<EEOK_Result>&, ResultCode);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_RejectInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Reject an invite from another player.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_RejectInvite")
	static UEOK_Sessions_RejectInvite* EOK_Sessions_RejectInvite(FString InviteId, FEOK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnRejectInviteCallback OnCallback;

private:
	FString Var_InviteId;
	FEOK_ProductUserId Var_LocalUserId;
	virtual void Activate() override;
	static void EOS_CALL OnRejectInviteCallback(const EOS_Sessions_RejectInviteCallbackInfo* Data);
};
