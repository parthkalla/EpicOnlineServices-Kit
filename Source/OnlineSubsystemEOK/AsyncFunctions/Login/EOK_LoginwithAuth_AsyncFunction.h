// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_LoginwithAuth_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_LoginwithAuthDelegate, FEOK_EpicAccountId, EpicUserID, FEOK_ProductUserId, ProductUserID, FString, Error);

UCLASS()
class OnlineSubsystemEOK_API UEOK_LoginwithAuth_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/*
		This function is used to login using EOK using the Auth Interface easily without having to worry about the underlying implementation.
		@param CredentialType: The login method to use.
		@param DisplayName: The display name of the user.
		@param Token: The token to use for login.

		Please note that the Auth Interface means that an EPIC ACCOUNT is required to login. This means you can use features that require an Epic Account like Friends, Overlay, etc.
	*/
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Epic Online Services-Kit V2 | Login", DisplayName = "Login using Auth Interface")
	static UEOK_LoginwithAuth_AsyncFunction* LoginWithAuth(TEnumAsByte<EEOK_ELoginCredentialType> CredentialType = EEOK_ELoginCredentialType::EOK_LCT_AccountPortal, TEnumAsByte<EEOK_EExternalCredentialType> ExternalCredentialType = EEOK_EExternalCredentialType::EOK_ECT_EPIC, FString Id = "", FString Token = "");

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Login")
	FEOK_LoginwithAuthDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | Login")
	FEOK_LoginwithAuthDelegate OnFailure;
private:
	EEOK_ELoginCredentialType Var_CredentialType;
	FString Var_Id;
	FString Var_Token;
	EEOK_EExternalCredentialType Var_ExternalCredentialType;
	virtual void Activate() override;
	void LoginCallback(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error);

};
