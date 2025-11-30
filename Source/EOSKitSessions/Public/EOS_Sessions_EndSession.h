// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_EndSession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSKit_OnEndSessionCallback, EEOSKitResult, ResultCode);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_EndSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Mark a session as ended
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_EndSession", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_EndSession* EOS_Sessions_EndSession(const FString& SessionName);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnEndSessionCallback OnCallback;

private:
	FString Var_SessionName;

	virtual void Activate() override;
	static void EOS_CALL OnEndSessionCallback(const EOS_Sessions_EndSessionCallbackInfo* Data);
};

