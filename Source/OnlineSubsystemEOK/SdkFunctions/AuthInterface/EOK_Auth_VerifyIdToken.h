// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
THIRD_PARTY_INCLUDES_START
#include <eos_auth.h>
THIRD_PARTY_INCLUDES_END
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Auth_VerifyIdToken.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Auth_VerifyIdTokenCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	TEnumAsByte<EEOK_Result> ResultCode;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString ApplicationId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString ClientId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString ProductId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString SandboxId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString DeploymentId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString DisplayName;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	bool bIsExternalAccountInfoPresent;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	TEnumAsByte<EEOK_EExternalAccountType> ExternalAccountIdType;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString ExternalAccountId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString ExternalAccountDisplayName;
	
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString Platform;

	FEOK_Auth_VerifyIdTokenCallbackInfo()
	{
		ResultCode = EEOK_Result::EOS_NotFound;
		ApplicationId = "";
		ClientId = "";
		ProductId = "";
		SandboxId = "";
		DeploymentId = "";
		DisplayName = "";
		bIsExternalAccountInfoPresent = false;
		ExternalAccountIdType = EEOK_EExternalAccountType::EOK_EAT_EPIC;
		ExternalAccountId = "";
		ExternalAccountDisplayName = "";
		Platform = "";
	}
	FEOK_Auth_VerifyIdTokenCallbackInfo(const EOS_Auth_VerifyIdTokenCallbackInfo& Data)
	{
		ResultCode = static_cast<EEOK_Result>(Data.ResultCode);
		ApplicationId = FString(UTF8_TO_TCHAR(Data.ApplicationId));
		ClientId = FString(UTF8_TO_TCHAR(Data.ClientId));
		ProductId = FString(UTF8_TO_TCHAR(Data.ProductId));
		SandboxId = FString(UTF8_TO_TCHAR(Data.SandboxId));
		DeploymentId = FString(UTF8_TO_TCHAR(Data.DeploymentId));
		DisplayName = FString(UTF8_TO_TCHAR(Data.DisplayName));
		if(Data.bIsExternalAccountInfoPresent == EOS_TRUE)
		{
			bIsExternalAccountInfoPresent = true;
		}
		else
		{
			bIsExternalAccountInfoPresent = false;
		}
		ExternalAccountIdType = static_cast<EEOK_EExternalAccountType>(Data.ExternalAccountIdType);
		ExternalAccountId = FString(UTF8_TO_TCHAR(Data.ExternalAccountId));
		ExternalAccountDisplayName = FString(UTF8_TO_TCHAR(Data.ExternalAccountDisplayName));
		Platform = FString(UTF8_TO_TCHAR(Data.Platform));
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Auth_OnVerifyIdTokenCallback, const FEOK_Auth_VerifyIdTokenCallbackInfo&, Data);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Auth_VerifyIdToken : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Verify a given ID token for authenticity and validity.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Auth Interface", DisplayName = "EOS_Auth_VerifyIdToken")
	static UEOK_Auth_VerifyIdToken* VerifyIdToken(FEOK_Auth_IdToken IdToken);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Auth_OnVerifyIdTokenCallback OnCallback;
private:
	FEOK_Auth_IdToken Var_IdToken;
	static void EOS_CALL OnVerifyIdTokenCallback(const EOS_Auth_VerifyIdTokenCallbackInfo* Data);
	virtual void Activate() override;

};
