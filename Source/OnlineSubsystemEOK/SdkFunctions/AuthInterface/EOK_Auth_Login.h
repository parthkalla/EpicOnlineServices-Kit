// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_auth_types.h"
#include <eos_base.h>
#include <eos_auth.h>
THIRD_PARTY_INCLUDES_END
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_Auth_Login.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Auth_LoginCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2")
	TEnumAsByte<EEOK_Result> Result = EEOK_Result::EOS_ServiceFailure;

	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2")
	FEOK_EpicAccountId LocalUserId;

	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2")
	FEOK_Auth_PinGrantInfo PinGrantInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2")
	FEOK_ContinuanceToken ContinuanceToken;

	UPROPERTY(BlueprintReadOnly, Category = "Epic Online Services-Kit V2")
	FEOK_EpicAccountId SelectedAccountId;

	FEOK_Auth_LoginCallbackInfo() = default;

	FEOK_Auth_LoginCallbackInfo(EOS_Auth_LoginCallbackInfo Data)
	{
		Result = static_cast<EEOK_Result>(Data.ResultCode);
		LocalUserId = Data.LocalUserId;
		PinGrantInfo = *Data.PinGrantInfo;
		ContinuanceToken = Data.ContinuanceToken;
		SelectedAccountId = Data.SelectedAccountId;
	}
};

USTRUCT(BlueprintType)
struct FEOK_Auth_LoginOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FEOK_Auth_Credentials Credentials;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2", meta=(Bitmask, BitmaskEnum = "/Script/OnlineSubsystemEOK.EEOK_EAuthScopeFlags"))
	int32 ScopeFlags;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	int64 LoginFlags;

	FEOK_Auth_LoginOptions()
	{
		ScopeFlags = EEOK_EAuthScopeFlags::EOK_AS_NoFlags;
		LoginFlags = 0;
	}
	EOS_Auth_LoginOptions ToEOSAuthLoginOptions()
	{
		EOS_Auth_LoginOptions Options;
		Options.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
		EOS_Auth_Credentials CredentialsRef = Credentials.GetValueAsEosType();
		Options.Credentials = &CredentialsRef;
		Options.ScopeFlags = static_cast<EOS_EAuthScopeFlags>(ScopeFlags);
		Options.LoginFlags = LoginFlags;
		return Options;
	}
	
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Auth_LoginCallback, const FEOK_Auth_LoginCallbackInfo&, Data);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Auth_Login : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Login/Authenticate with user credentials.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName = "EOS_Auth_Login")
	static UEOK_Auth_Login* Login(FEOK_Auth_LoginOptions Options);
	
	UPROPERTY(BlueprintAssignable)
	FEOK_Auth_LoginCallback OnCallback;
	
private:
	FEOK_Auth_LoginOptions Var_Options;
	virtual void Activate() override;
	static void EOS_CALL Internal_OnLoginComplete(const EOS_Auth_LoginCallbackInfo* Data);
};
