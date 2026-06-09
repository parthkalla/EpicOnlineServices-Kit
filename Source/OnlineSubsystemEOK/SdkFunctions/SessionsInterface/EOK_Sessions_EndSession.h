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
#include "EOK_Sessions_EndSession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnEndSessionCallback, const TEnumAsByte<EEOK_Result>&, ResultCode);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_EndSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Mark a session as ended, making it unavailable to find if "join in progress" was disabled. The session may be started again if desired
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_EndSession")
	static UEOK_Sessions_EndSession* EOK_Sessions_EndSession(FString SessionName);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnEndSessionCallback OnCallback;

private:
	FString Var_SessionName;
	virtual void Activate() override;
	static void EOS_CALL OnEndSessionCallback(const EOS_Sessions_EndSessionCallbackInfo* Data);
	
};
