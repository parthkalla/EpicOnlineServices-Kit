// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
THIRD_PARTY_INCLUDES_START
#include <eos_auth.h>
THIRD_PARTY_INCLUDES_END
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Auth_Logout.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLogoutCallback, TEnumAsByte<EEOK_Result>, ResultCode, FEOK_EpicAccountId, EpicAccountId);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Auth_Logout : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Signs the player out of the online service.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName = "EOS_Auth_Logout")
	static UEOK_Auth_Logout* Logout(FEOK_EpicAccountId EpicAccountId);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FOnLogoutCallback OnCallback;
private:
	FEOK_EpicAccountId Var_EpicAccountId;
	virtual void Activate() override;
	static void EOS_CALL OnLogoutCallback(const EOS_Auth_LogoutCallbackInfo* Data);
};
