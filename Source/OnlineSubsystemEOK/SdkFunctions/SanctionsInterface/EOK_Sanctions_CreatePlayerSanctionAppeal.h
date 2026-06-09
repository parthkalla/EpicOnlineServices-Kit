// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "eos_sanctions.h"
#include "eos_sanctions_types.h"
#include "OnlineSubsystemEOS.h"
#include "EOK_Sanctions_CreatePlayerSanctionAppeal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Sanctions_CreatePlayerSanctionAppealComplete, const FEOK_ProductUserId&, LocalUserId, const TEnumAsByte<EEOK_Result>&, Result);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sanctions_CreatePlayerSanctionAppeal : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Create a sanction appeal on behalf of a local user. Note that for creating the sanction appeal you'll need the sanction reference id, which is available through CopyPlayerSanctionByIndex.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sanctions Interface", DisplayName="EOS_Sanctions_CreatePlayerSanctionAppeal")
	static UEOK_Sanctions_CreatePlayerSanctionAppeal* EOK_Sanctions_CreatePlayerSanctionAppeal(FEOK_ProductUserId LocalUserId, const TEnumAsByte<EEOK_ESanctionAppealReason>& AppealReason, const FString& ReferenceId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sanctions Interface")
	FEOK_Sanctions_CreatePlayerSanctionAppealComplete OnCallback;
	
private:
	void Activate() override;
	static void EOS_CALL Internal_OnCreatePlayerSanctionAppealComplete(const EOS_Sanctions_CreatePlayerSanctionAppealCallbackInfo* Data);
	FEOK_ProductUserId Var_LocalUserId;
	TEnumAsByte<EEOK_ESanctionAppealReason> Var_AppealReason;
	FString Var_ReferenceId;
	
};
