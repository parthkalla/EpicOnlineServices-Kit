// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_SendInvite.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSKit_OnSendInviteCallback, EEOSKitResult, ResultCode);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_SendInvite : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Send an invite to another player
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_SendInvite", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_SendInvite* EOS_Sessions_SendInvite(const FString& SessionName, const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& TargetUserId);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnSendInviteCallback OnCallback;

private:
	FString Var_SessionName;
	FEOSKitProductUserId Var_LocalUserId;
	FEOSKitProductUserId Var_TargetUserId;

	virtual void Activate() override;
	static void EOS_CALL OnSendInviteCallback(const EOS_Sessions_SendInviteCallbackInfo* Data);
};

