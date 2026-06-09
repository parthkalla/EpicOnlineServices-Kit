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
#include "EOK_Sessions_SendInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnSendInviteCallback, const TEnumAsByte<EEOK_Result>&, ResultCode);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_SendInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	//Send an invite to another player. User must have created the session or be registered in the session or else the call will fail
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_SendInvite")
	static UEOK_Sessions_SendInvite* EOK_Sessions_SendInvite(FString SessionName, FEOK_ProductUserId LocalUserId, FEOK_ProductUserId TargetUserId);

	 UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnSendInviteCallback OnCallback;

private:
	FString Var_SessionName;
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_ProductUserId Var_TargetUserId;
	virtual void Activate() override;
	static void EOS_CALL OnSendInviteCallback(const EOS_Sessions_SendInviteCallbackInfo* Data);
};
