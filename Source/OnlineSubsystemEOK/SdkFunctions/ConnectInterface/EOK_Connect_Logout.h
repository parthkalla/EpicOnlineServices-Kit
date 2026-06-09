// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Connect_Logout.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_Connect_Logout_Delegate, const FEOK_ProductUserId&, ProductUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Connect_Logout : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Logout a currently logged in user. NOTE: Access tokens for Product User IDs cannot be revoked. This operation really just cleans up state for the Product User ID and locally discards any associated access token.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_Logout")
	static UEOK_Connect_Logout* EOK_Connect_Logout(FEOK_ProductUserId ProductUserId);
	
	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_Connect_Logout_Delegate OnCallback;

	static void OnLogoutCallback(const EOS_Connect_LogoutCallbackInfo* Data);

private:
	FEOK_ProductUserId Var_ProductUserId;
	virtual void Activate() override;
	
};
