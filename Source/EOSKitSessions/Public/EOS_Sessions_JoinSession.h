// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_JoinSession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSKit_OnJoinSessionCallback, EEOSKitResult, ResultCode);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_JoinSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Join a session, creating a local session under a given session name
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_JoinSession", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_JoinSession* EOK_Sessions_JoinSession(const FString& SessionName, const FEOSKitProductUserId& LocalUserId, const FEOSKitHSessionDetails& SessionHandle, bool bPresenceEnabled);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnJoinSessionCallback OnCallback;

private:
	FString Var_SessionName;
	FEOSKitProductUserId Var_LocalUserId;
	FEOSKitHSessionDetails Var_SessionHandle;
	bool Var_bPresenceEnabled;

	virtual void Activate() override;
	static void EOS_CALL OnJoinSessionCallback(const EOS_Sessions_JoinSessionCallbackInfo* Data);
};

