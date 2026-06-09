// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sessions.h"
#include "eos_sessions_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_Sessions_StartSession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnStartSessionCallback, const TEnumAsByte<EEOK_Result>&, ResultCode);
UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_StartSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//Mark a session as started, making it unable to find if session properties indicate "join in progress" is not available
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_StartSession")
	static UEOK_Sessions_StartSession* EOK_Sessions_StartSession(FString SessionName);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnStartSessionCallback OnCallback;

private:
	FString Var_SessionName;
	virtual void Activate() override;
	static void EOS_CALL OnStartSessionCallback(const EOS_Sessions_StartSessionCallbackInfo* Data);
	
	
};
