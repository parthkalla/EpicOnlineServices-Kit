// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
THIRD_PARTY_INCLUDES_START
#include <eos_auth.h>
THIRD_PARTY_INCLUDES_END
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Auth_QueryIdToken.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_Auth_OnQueryIdTokenCallback, TEnumAsByte<EEOK_Result>, ResultCode, FEOK_EpicAccountId, LocalUserId, FEOK_EpicAccountId, TargetAccountId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Auth_QueryIdToken : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Query the backend for an ID token that describes one of the merged account IDs of a local authenticated user. The ID token can be used to impersonate a merged account ID when communicating with online services. An ID token for the selected account ID of a locally authenticated user will always be readily available and does not need to be queried explicitly.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName = "EOS_Auth_QueryIdToken")
	static UEOK_Auth_QueryIdToken* QueryIdToken(FEOK_EpicAccountId LocalUserId, FEOK_EpicAccountId TargetAccountId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Auth_OnQueryIdTokenCallback OnCallback;
private:
	FEOK_EpicAccountId Var_LocalUserId;
	FEOK_EpicAccountId Var_TargetAccountId;
	static void EOS_CALL OnQueryIdTokenCallback(const EOS_Auth_QueryIdTokenCallbackInfo* Data);
	virtual void Activate() override;
	
};
