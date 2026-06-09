// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "eos_sanctions.h"
#include "eos_sanctions_types.h"
#include "OnlineSubsystemEOS.h"
#include "EOK_Sanctions_QueryActivePlayerSanctions.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_Sanctions_QueryActivePlayerSanctionsComplete, const FEOK_ProductUserId&, LocalUserId, const FEOK_ProductUserId&, TargetUserId, const TEnumAsByte<EEOK_Result>&, Result);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sanctions_QueryActivePlayerSanctions : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Start an asynchronous query to retrieve any active sanctions for a specified user. Call EOS_Sanctions_GetPlayerSanctionCount and EOS_Sanctions_CopyPlayerSanctionByIndex to retrieve the data.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sanctions Interface", DisplayName="EOS_Sanctions_QueryActivePlayerSanctions")
	static UEOK_Sanctions_QueryActivePlayerSanctions* EOK_Sanctions_QueryActivePlayerSanctions(FEOK_ProductUserId LocalUserId, FEOK_ProductUserId TargetUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sanctions Interface")
	FEOK_Sanctions_QueryActivePlayerSanctionsComplete OnCallback;
private:
	virtual void Activate() override;
	static void EOS_CALL Internal_OnQueryActivePlayerSanctionsComplete(const EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo* Data);
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_ProductUserId Var_TargetUserId;
	
};
