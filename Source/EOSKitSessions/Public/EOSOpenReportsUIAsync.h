// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitSharedTypes.h"
#include "EOSOpenReportsUIAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOpenReportsUIComplete, EEOSResult, Result);

/**
 * Async node to open the report player UI in the social overlay
 */
UCLASS()
class EOSKITSESSIONS_API UEOSOpenReportsUIAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Open the report player UI in the social overlay
	 * @param WorldContextObject - World context
	 * @param LocalUserId - Epic Account ID of the local user requesting the report
	 * @param TargetUserId - Epic Account ID of the user to be reported
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Reports",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		DisplayName = "Open Report Player UI")
	static UEOSOpenReportsUIAsync* OpenReportsUI(
		UObject* WorldContextObject,
		const FEOSKitEpicAccountId& LocalUserId,
		const FEOSKitEpicAccountId& TargetUserId
	);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Reports")
	FOnOpenReportsUIComplete OnComplete;

private:
	virtual void Activate() override;

	static void EOS_CALL OnOpenReportsUICompleteCallback(const EOS_UI_OnShowReportPlayerCallbackInfo* Data);

	UPROPERTY()
	UObject* WorldContextObject;

	FEOSKitEpicAccountId LocalUserId;
	FEOSKitEpicAccountId TargetUserId;
};

