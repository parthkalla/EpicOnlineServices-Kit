// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_QueryInvites.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSKit_OnQueryInvitesCallback, EEOSKitResult, ResultCode, const FEOSKitProductUserId&, LocalUserId);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_QueryInvites : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Retrieve all existing invites for a single user
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_QueryInvites", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_QueryInvites* EOK_Sessions_QueryInvites(const FEOSKitProductUserId& LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnQueryInvitesCallback OnCallback;

private:
	FEOSKitProductUserId Var_LocalUserId;

	virtual void Activate() override;
	static void EOS_CALL OnQueryInvitesCallback(const EOS_Sessions_QueryInvitesCallbackInfo* Data);
};

