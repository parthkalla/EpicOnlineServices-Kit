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
#include "EOK_Sessions_JoinSession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOK_OnJoinSessionCallback, const TEnumAsByte<EEOK_Result>&, ResultCode);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Sessions_JoinSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	//Join a session, creating a local session under a given session name. Backend will validate various conditions to make sure it is possible to join the session.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface", DisplayName="EOS_Sessions_JoinSession")
	static UEOK_Sessions_JoinSession* EOK_Sessions_JoinSession(FString SessionName, FEOK_ProductUserId LocalUserId, FEOK_HSessionDetails SessionHandle, bool bPresenceEnabled);
	
	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sessions Interface")
	FEOK_OnJoinSessionCallback OnCallback;
private:
	FString Var_SessionName;
	FEOK_ProductUserId Var_LocalUserId;
	FEOK_HSessionDetails Var_SessionHandle;
	bool Var_bPresenceEnabled;
	virtual void Activate() override;
	static void EOS_CALL OnJoinSessionCallback(const EOS_Sessions_JoinSessionCallbackInfo* Data);
};
