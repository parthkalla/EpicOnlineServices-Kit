// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "eos_connect_types.h"
#include "eos_connect.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_LinkAccount_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLinkAccountCallback, TEnumAsByte<EEOK_Result>, Result);
UCLASS()
class OnlineSubsystemEOK_API UEOK_LinkAccount_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | Connect | Link Account")
	static UEOK_LinkAccount_AsyncFunction* LinkAccount(const FString& LocalProductUserId, const FEOK_ContinuanceToken& ContinuanceToken);
	
	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FOnLinkAccountCallback OnCallback;

private:
	FString Var_LocalProductUserId;
	FEOK_ContinuanceToken Var_ContinuanceToken;
	static void EOS_CALL OnLinkAccountCallback(const EOS_Connect_LinkAccountCallbackInfo* Data);
	virtual void Activate() override;
};
