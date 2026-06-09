// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_ui.h"
#include "eos_ui_types.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_OpenReportsUI_AsyncFunction.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReportsUIDelegate);

UCLASS()
class OnlineSubsystemEOK_API UEOK_OpenReportsUI_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	//PUID's'
	FString LocalReporterPUID;
	FString TargetPlayerPUID;

	//Delegates
	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 || Reports")
	FReportsUIDelegate Success;

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 || Reports")
	FReportsUIDelegate Failure;

	UFUNCTION(BlueprintCallable, DisplayName = "Show Epic Player Report UI", meta = (BlueprintInternalUseOnly = "true"), Category = "Epic Online Services-Kit V2 || Reports")
	static UEOK_OpenReportsUI_AsyncFunction* ShowEOKPlayerReportUIAsyncFunction(FString LocalReporterPUID, FString TargetPlayerPUID);

	void SendReportFunc();

	static void EOS_CALL SendReportFuncCallback(const EOS_UI_OnShowReportPlayerCallbackInfo* Data);

	void ResultFaliure();

	void ResultSuccess();

	void Activate() override;
};
