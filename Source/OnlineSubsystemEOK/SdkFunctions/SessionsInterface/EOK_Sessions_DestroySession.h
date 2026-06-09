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
#include "EOK_Sessions_DestroySession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnDestroySessionCallback, const TEnumAsByte<EEOK_Result>&, ResultCode);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_DestroySession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	//Destroy a session given a session name
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_DestroySession")
	static UEOK_Sessions_DestroySession* EOK_Sessions_DestroySession(FString SessionName);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2")
	FEOK_OnDestroySessionCallback OnCallback;
private:
	FString Var_SessionName;
	virtual void Activate() override;
	static void EOS_CALL OnDestroySessionCallback(const EOS_Sessions_DestroySessionCallbackInfo* Data);
};
