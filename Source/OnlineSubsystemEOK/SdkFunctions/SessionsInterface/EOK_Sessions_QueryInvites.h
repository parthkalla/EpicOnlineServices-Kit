// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Async/TaskGraphInterfaces.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sessions.h"
#include "eos_sessions_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_Sessions_QueryInvites.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_OnQueryInvitesCallback, const TEnumAsByte<EEOK_Result>&, ResultCode, const FEOK_ProductUserId&, LocalUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_QueryInvites : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Retrieve all existing invites for a single user
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_QueryInvites")
	static UEOK_Sessions_QueryInvites* EOK_Sessions_QueryInvites(FEOK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnQueryInvitesCallback OnCallback;

private:
	FEOK_ProductUserId Var_LocalUserId;
	virtual void Activate() override;
	static void EOS_CALL OnQueryInvitesCallback(const EOS_Sessions_QueryInvitesCallbackInfo* Data);
	
};
