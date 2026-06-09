// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "eos_auth.h"
#include "eos_auth_types.h"

#include "EOK_GetIdToken_AsyncFunction.generated.h"

USTRUCT(BlueprintType, Category = "Epic Online Services-Kit V2 || Login")
struct FEOKCopyAuthToken
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 || Login")
	FString EpicAccountId = "";

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 || Login")
	FString IdToken_JWT = "";

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetIdToken_Delegate, const FEOKCopyAuthToken, EOKCopyAuthToken);


UCLASS()
class OnlineSubsystemEOK_API UEOK_GetIdToken_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	FString Var_EpicAccountId;

	EOS_Auth_IdToken* Var_IdToken;

	//Delegates
	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 || Login")
	FGetIdToken_Delegate Success;

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 || Login")
	FGetIdToken_Delegate Failure;
	
	UFUNCTION(BlueprintCallable, DisplayName = "Get EOK Auth Token", meta = (BlueprintInternalUseOnly = "true", ToolTip = "Get id token, often used for OpenID. Returns an ID token as a JSON Web Token (JWT) in string format. Only works with Epic accounts."), Category = "Epic Online Services-Kit V2 || Login")
	static UEOK_GetIdToken_AsyncFunction* GetEOKAuthToken(FString EpicAccountId);

	void GetAuthToken();

	void Activate() override;
};
