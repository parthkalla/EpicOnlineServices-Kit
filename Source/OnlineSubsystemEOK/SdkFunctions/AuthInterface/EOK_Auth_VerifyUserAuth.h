// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
THIRD_PARTY_INCLUDES_START
#include <eos_auth.h>
THIRD_PARTY_INCLUDES_END
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Auth_VerifyUserAuth.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Auth_OnVerifyUserAuthCallback, TEnumAsByte<EEOK_Result>, ResultCode);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Auth_VerifyUserAuth : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Contact the backend service to verify validity of an existing user auth token. This function is intended for server-side use only.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName = "EOS_Auth_VerifyUserAuth")
	static UEOK_Auth_VerifyUserAuth* VerifyUserAuth(FEOK_Auth_Token AuthToken);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Auth_OnVerifyUserAuthCallback OnCallback;

private:
	FEOK_Auth_Token Var_AuthToken;
	static void EOS_CALL OnVerifyUserAuthCallback(const EOS_Auth_VerifyUserAuthCallbackInfo* Data);
	virtual void Activate() override;
};
