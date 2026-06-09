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
#include "EOK_SessionSearch_Find.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnSessionSearch_FindCallback, const TEnumAsByte<EEOK_Result>&, ResultCode);

UCLASS()
class OnlineSubsystemEOK_API UEOK_SessionSearch_Find : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Find sessions matching the search criteria setup via this session search handle. When the operation completes, this handle will have the search results that can be parsed
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_SessionSearch_Find")
	static UEOK_SessionSearch_Find* EOK_SessionSearch_Find(FEOK_HSessionSearch SessionSearchHandle, FEOK_ProductUserId LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnSessionSearch_FindCallback OnCallback;

private:
	FEOK_HSessionSearch Var_SessionSearchHandle;
	FEOK_ProductUserId Var_LocalUserId;
	virtual void Activate() override;
	static void EOS_CALL OnSessionSearch_FindCallback(const EOS_SessionSearch_FindCallbackInfo* Data);
};
