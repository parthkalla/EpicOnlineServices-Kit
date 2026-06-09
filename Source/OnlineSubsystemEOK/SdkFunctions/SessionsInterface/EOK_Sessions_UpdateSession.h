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
#include "EOK_Sessions_UpdateSession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOK_OnUpdateSessionCallback, const TEnumAsByte<EEOK_Result>&, ResultCode, const FString&, SessionName, const FString&, SessionId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_UpdateSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Update a session given a session modification handle created by EOS_Sessions_CreateSessionModification or EOS_Sessions_UpdateSessionModification
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_UpdateSession")
	static UEOK_Sessions_UpdateSession* EOK_Sessions_UpdateSession(FEOK_HSessionModification SessionModificationHandle);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnUpdateSessionCallback OnCallback;

private:
	FEOK_HSessionModification Var_SessionModificationHandle;
	virtual void Activate() override;
	static void EOS_CALL OnUpdateSessionCallback(const EOS_Sessions_UpdateSessionCallbackInfo* Data);
};
