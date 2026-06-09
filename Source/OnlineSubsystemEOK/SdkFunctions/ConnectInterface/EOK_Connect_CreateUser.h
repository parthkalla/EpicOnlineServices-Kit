// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Connect_CreateUser.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreateUserCallback, EEOK_Result, ResultCode, FEOK_ProductUserId, LocalUserId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Connect_CreateUser : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Create an account association with the Epic Online Service as a product user given their external auth credentials.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Connect Interface", DisplayName="EOS_Connect_CreateUser")
	static UEOK_Connect_CreateUser* CreateUser(FEOK_ContinuanceToken ContinuanceToken);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FOnCreateUserCallback OnCallback;

private:
	virtual void Activate() override;
	FEOK_ContinuanceToken Var_ContinuanceToken;
	static void EOS_CALL OnCreateUserCallback(const EOS_Connect_CreateUserCallbackInfo* Data);
};
