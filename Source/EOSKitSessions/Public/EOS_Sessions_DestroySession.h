// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_DestroySession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSKit_OnDestroySessionCallback, EEOSKitResult, ResultCode);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_DestroySession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Destroy a session given a session name
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_DestroySession", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_DestroySession* EOK_Sessions_DestroySession(const FString& SessionName);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnDestroySessionCallback OnCallback;

private:
	FString Var_SessionName;

	virtual void Activate() override;
	static void EOS_CALL OnDestroySessionCallback(const EOS_Sessions_DestroySessionCallbackInfo* Data);
};

