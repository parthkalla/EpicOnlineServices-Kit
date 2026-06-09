// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "eos_presence.h"
#include "OnlineSubsystemEOS.h"
#include "EOK_Presence_QueryPresence.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_Presence_QueryPresenceComplete, const FEOK_EpicAccountId&, LocalUserId, const FEOK_EpicAccountId&, TargetUserId, const TEnumAsByte<EEOK_Result>&, Result);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Presence_QueryPresence : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Query a user's presence. This must complete successfully before CopyPresence will have valid results. If HasPresence returns true for a remote user, this does not need to be called.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_QueryPresence")
	static UEOK_Presence_QueryPresence* EOK_Presence_QueryPresence(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetUserId);

	UPROPERTY(BlueprintAssignable)
	FEOK_Presence_QueryPresenceComplete OnCallback;

private:
	virtual void Activate() override;
	static void EOS_CALL Internal_OnQueryPresenceComplete(const EOS_Presence_QueryPresenceCallbackInfo* Data);
	FEOK_EpicAccountId Var_LocalUserId;
	FEOK_EpicAccountId Var_TargetUserId;
};
