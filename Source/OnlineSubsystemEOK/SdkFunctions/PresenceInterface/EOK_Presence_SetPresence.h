// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "eos_presence.h"
#include "OnlineSubsystemEOS.h"
#include "EOK_Presence_SetPresence.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Presence_SetPresenceComplete, const FEOK_EpicAccountId&, LocalUserId, const TEnumAsByte<EEOK_Result>&, Result);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Presence_SetPresence : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Sets your new presence with the data applied to a PresenceModificationHandle. The PresenceModificationHandle can be released safely after calling this function.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Presence Interface", DisplayName="EOS_Presence_SetPresence")
	static UEOK_Presence_SetPresence* EOK_Presence_SetPresence(FEOK_EpicAccountId LocalUserId, FEOK_HPresenceModification PresenceModificationHandle);

	UPROPERTY(BlueprintAssignable)
	FEOK_Presence_SetPresenceComplete OnCallback;

private:
	virtual void Activate() override;
	static void EOS_CALL Internal_OnSetPresenceComplete(const EOS_Presence_SetPresenceCallbackInfo* Data);
	FEOK_EpicAccountId Var_LocalUserId;
	FEOK_HPresenceModification Var_PresenceModificationHandle;
};
