// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_auth_types.h"
#include "eos_auth.h"
THIRD_PARTY_INCLUDES_END
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_Auth_LinkAccount.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Auth_LinkAccountCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	TEnumAsByte<EEOK_Result> ResultCode;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FEOK_EpicAccountId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FEOK_Auth_PinGrantInfo PinGrantInfo;
	
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FEOK_EpicAccountId SelectedAccountId;

	FEOK_Auth_LinkAccountCallbackInfo()
	{
		ResultCode = EEOK_Result::EOS_Success;
	}

	FEOK_Auth_LinkAccountCallbackInfo(const EOS_Auth_LinkAccountCallbackInfo* data)
	{
		ResultCode = static_cast<EEOK_Result>(data->ResultCode);
		LocalUserId = data->LocalUserId;
		PinGrantInfo = *data->PinGrantInfo;
		SelectedAccountId = data->SelectedAccountId;
	}
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Auth_OnLinkAccountCallback, FEOK_Auth_LinkAccountCallbackInfo, Data);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Auth_LinkAccount : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/*
	 *Link external account by continuing previous login attempt with a continuance token. The user will be presented with Epic Accounts onboarding flow managed by the SDK. On success, the user will be logged in at the completion of this action. This will commit this external account to the Epic Account and cannot be undone in the SDK.
	*/
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName = "EOS_Auth_LinkAccount")
	static UEOK_Auth_LinkAccount* LinkAccount(FEOK_ContinuanceToken ContinuanceToken, FEOK_EpicAccountId LocalUserId, TEnumAsByte<EEOK_ELinkAccountFlags> LinkAccountFlags);
	
	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Auth_OnLinkAccountCallback OnCallback;
	
private:
	FEOK_ContinuanceToken Var_ContinuanceToken;
	FEOK_EpicAccountId Var_LocalUserId;
	TEnumAsByte<EEOK_ELinkAccountFlags> Var_LinkAccountFlags;
	virtual void Activate() override;
	static void EOS_CALL OnLinkAccountCallback(const EOS_Auth_LinkAccountCallbackInfo* Data);
};
