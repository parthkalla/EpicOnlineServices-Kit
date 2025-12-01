// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_SessionSearch_Find.generated.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sessions_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSKit_OnSessionSearch_FindCallback, EEOSKitResult, ResultCode);

UCLASS()
class EOSKITSESSIONS_API UEOS_SessionSearch_Find : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Find sessions matching the search criteria
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_SessionSearch_Find", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_SessionSearch_Find* EOS_SessionSearch_Find(const FEOSKitHSessionSearch& SessionSearchHandle, const FEOSKitProductUserId& LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnSessionSearch_FindCallback OnCallback;

private:
	FEOSKitHSessionSearch Var_SessionSearchHandle;
	FEOSKitProductUserId Var_LocalUserId;

	virtual void Activate() override;
#if WITH_EOS_SDK
	static void EOS_CALL OnSessionSearch_FindCallback(const EOS_SessionSearch_FindCallbackInfo* Data);
#endif
};

