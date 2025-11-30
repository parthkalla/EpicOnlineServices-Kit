// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSessionsSDKShared.h"
#include "EOSKitSubsystem.h"
#include "EOS_Sessions_UpdateSession.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEOSKit_OnUpdateSessionCallback, const TEnumAsByte<EEOSKitResult>&, ResultCode, const FString&, SessionName, const FString&, SessionId);

UCLASS()
class EOSKITSESSIONS_API UEOS_Sessions_UpdateSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Update a session given a session modification handle
	UFUNCTION(BlueprintCallable, Category = "EOSKit | SDK Functions | Sessions Interface", DisplayName = "EOS_Sessions_UpdateSession", meta = (BlueprintInternalUseOnly = "true"))
	static UEOS_Sessions_UpdateSession* EOS_Sessions_UpdateSession(const FEOSKitHSessionModification& SessionModificationHandle);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit | SDK Functions | Sessions Interface")
	FEOSKit_OnUpdateSessionCallback OnCallback;

private:
	FEOSKitHSessionModification Var_SessionModificationHandle;

	virtual void Activate() override;
	static void EOS_CALL OnUpdateSessionCallback(const EOS_Sessions_UpdateSessionCallbackInfo* Data);
};

