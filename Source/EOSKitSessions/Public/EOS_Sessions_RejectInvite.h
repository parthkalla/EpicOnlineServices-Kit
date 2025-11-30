// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_RejectInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSKit_OnRejectInviteCallback, EEOSKitResult, ResultCode);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_RejectInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Reject an invite from another player
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_RejectInvite", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_RejectInvite* EOS_Sessions_RejectInvite(const FString& InviteId, const FEOSKitProductUserId& LocalUserId);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnRejectInviteCallback OnCallback;

private:
	FString Var_InviteId;
	FEOSKitProductUserId Var_LocalUserId;

	virtual void Activate() override;
	static void EOS_CALL OnRejectInviteCallback(const EOS_Sessions_RejectInviteCallbackInfo* Data);
};

