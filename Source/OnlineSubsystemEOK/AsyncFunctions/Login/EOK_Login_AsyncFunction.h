// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "OnlineSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "EOK_Login_AsyncFunction.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FResponsDelegate, FEOK_ProductUserId, ProductUserID, FString, Error);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Login_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintAssignable)
	FResponsDelegate OnSuccess;
	
	UPROPERTY(BlueprintAssignable)
	FResponsDelegate OnFail;


	/*
	This C++ method logs in a user to an online subsystem using the selected method and sets up a callback function to handle the login response.
	Documentation link: https://betide-studio.gitbook.io/eos-integration-kit/authentication/
	For Input Parameters, please refer to the documentation link above.
	
	Following Methods are coming with upcoming build - Apple, Discord, Oculus, OpenID
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Login using Connect Interface", meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Login")
	static UEOK_Login_AsyncFunction* LoginUsingConnectInterface(TEnumAsByte<EEOK_EExternalCredentialType> LoginMethod = EEOK_EExternalCredentialType::EOK_ECT_DEVICEID_ACCESS_TOKEN , FString DisplayName = "", FString Token = "");

private:
	EEOK_EExternalCredentialType LoginMethod;
	FString DisplayName;
	FString Token;
	IOnlineSubsystem* AppleSubsystem;
	virtual void Activate() override;
	void LoginCallback(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error);
	
};
