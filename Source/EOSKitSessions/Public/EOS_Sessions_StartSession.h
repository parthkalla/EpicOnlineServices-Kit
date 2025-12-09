// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_StartSession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSKit_OnStartSessionCallback, EEOSKitResult, ResultCode);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_StartSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Mark a session as started
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_StartSession", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UEOS_Sessions_StartSession* EOK_Sessions_StartSession(UObject* WorldContextObject, const FString& SessionName);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnStartSessionCallback OnCallback;

private:
	FString Var_SessionName;
	TObjectPtr<UObject> CachedWorldContextObject;

	virtual void Activate() override;
	static void EOS_CALL OnStartSessionCallback(const EOS_Sessions_StartSessionCallbackInfo* Data);
};

