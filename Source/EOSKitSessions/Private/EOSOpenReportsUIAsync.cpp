// Copyright (C) 2024, All Rights Reserved.

#include "EOSOpenReportsUIAsync.h"
#include "EOSKitSubsystem.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_ui.h"
#include "eos_ui_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"

UEOSOpenReportsUIAsync* UEOSOpenReportsUIAsync::OpenReportsUI(
	UObject* WorldContextObject,
	const FEOSKitEpicAccountId& LocalUserId,
	const FEOSKitEpicAccountId& TargetUserId)
{
	UEOSOpenReportsUIAsync* BlueprintNode = NewObject<UEOSOpenReportsUIAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->LocalUserId = LocalUserId;
	BlueprintNode->TargetUserId = TargetUserId;
	return BlueprintNode;
}

void UEOSOpenReportsUIAsync::Activate()
{
	Super::Activate();

	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSOpenReportsUIAsync: WorldContextObject is null"));
		OnComplete.Broadcast(EEOSResult::EOS_InvalidParameters);
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSOpenReportsUIAsync: GameInstance is null"));
		OnComplete.Broadcast(EEOSResult::EOS_InvalidParameters);
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSOpenReportsUIAsync: EOSKitSubsystem or PlatformHandle is null"));
		OnComplete.Broadcast(EEOSResult::EOS_NotConfigured);
		SetReadyToDestroy();
		return;
	}

	EOS_HUI UIHandle = EOS_Platform_GetUIInterface(EOSKitSubsystem->GetPlatformHandle());
	if (!UIHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSOpenReportsUIAsync: Failed to get UI Handle"));
		OnComplete.Broadcast(EEOSResult::EOS_NotConfigured);
		SetReadyToDestroy();
		return;
	}

	EOS_UI_ShowReportPlayerOptions Options = {};
	Options.ApiVersion = EOS_UI_SHOWREPORTPLAYER_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	EOS_UI_ShowReportPlayer(
		UIHandle,
		&Options,
		this,
		&UEOSOpenReportsUIAsync::OnOpenReportsUICompleteCallback
	);
}

void EOS_CALL UEOSOpenReportsUIAsync::OnOpenReportsUICompleteCallback(const EOS_UI_OnShowReportPlayerCallbackInfo* CallbackInfo)
{

	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSOpenReportsUIAsync* Self = static_cast<UEOSOpenReportsUIAsync*>(const_cast<void*>(CallbackInfo->ClientData));
	EEOSResult Result = ConvertEOSResultToEEOSResult(CallbackInfo->ResultCode);

	if (Result == EEOSResult::EOS_Success)
	{
		UE_LOG(LogTemp, Log, TEXT("EOSOpenReportsUIAsync: Report UI opened successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSOpenReportsUIAsync: Failed to open report UI with result: %d"), 
			static_cast<int32>(Result));
	}

	Self->OnComplete.Broadcast(Result);
	Self->SetReadyToDestroy();
}

