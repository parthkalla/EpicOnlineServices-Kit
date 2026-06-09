// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Connect_VerifyIdToken.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Connect_VerifyIdTokenCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	TEnumAsByte<EEOK_Result> Result;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FEOK_ProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	bool bIsAccountInfoPresent;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	TEnumAsByte<EEOK_EExternalAccountType> AccountType;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString AccountId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString Platform;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString DeviceType;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString ClientId;
	
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString ProductId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString SandboxId;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2")
	FString DeploymentId;

	FEOK_Connect_VerifyIdTokenCallbackInfo()
	{
		Result = EEOK_Result::EOS_Success;
		LocalUserId = FEOK_ProductUserId();
		bIsAccountInfoPresent = false;
		AccountType = EEOK_EExternalAccountType::EOK_EAT_EPIC;
		AccountId = "";
		Platform = "";
		DeviceType = "";
		ClientId = "";
		ProductId = "";
		SandboxId = "";
		DeploymentId = "";
	}
	FEOK_Connect_VerifyIdTokenCallbackInfo(const EOS_Connect_VerifyIdTokenCallbackInfo* Data)
	{
		Result = static_cast<EEOK_Result>(Data->ResultCode);
		LocalUserId = Data->ProductUserId;
		if(Data->bIsAccountInfoPresent == EOS_TRUE)
		{
			bIsAccountInfoPresent = true;
		}
		else
		{
			bIsAccountInfoPresent = false;
		}
		AccountType = static_cast<EEOK_EExternalAccountType>(Data->AccountIdType);
		AccountId = Data->AccountId;
		Platform = Data->Platform;
		DeviceType = Data->DeviceType;
		ClientId = Data->ClientId;
		ProductId = Data->ProductId;
		SandboxId = Data->SandboxId;
		DeploymentId = Data->DeploymentId;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Connect_VerifyIdToken_Delegate, const FEOK_Connect_VerifyIdTokenCallbackInfo&, Data);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Connect_VerifyIdToken : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Verify a given ID token for authenticity and validity.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_VerifyIdToken")
	static UEOK_Connect_VerifyIdToken* EOK_Connect_VerifyIdToken(FEOK_Connect_IdToken IdToken);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Connect_VerifyIdToken_Delegate OnCallback;
private:
	static void OnVerifyIdTokenCallback(const EOS_Connect_VerifyIdTokenCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_Connect_IdToken Var_IdToken;
};
