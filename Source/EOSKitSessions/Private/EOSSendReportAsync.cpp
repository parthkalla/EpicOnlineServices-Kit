// Copyright (C) 2024, All Rights Reserved.

#include "EOSSendReportAsync.h"
#include "EOSKitSubsystem.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_reports.h"
#include "eos_reports_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "EOSKitSharedTypes.h"

UEOSSendReportAsync* UEOSSendReportAsync::SendReport(
	UObject* WorldContextObject,
	const FEOSKitProductUserId& ReporterUserId,
	const FEOSKitProductUserId& ReportedUserId,
	EEOSKitPlayerReportCategory Category,
	const FString& Message,
	const FString& Context)
{
	UEOSSendReportAsync* BlueprintNode = NewObject<UEOSSendReportAsync>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->ReporterUserId = ReporterUserId;
	BlueprintNode->ReportedUserId = ReportedUserId;
	BlueprintNode->Category = Category;
	BlueprintNode->Message = Message;
	BlueprintNode->Context = Context.IsEmpty() ? TEXT("{}") : Context;
	return BlueprintNode;
}

void UEOSSendReportAsync::Activate()
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSSendReportAsync: WorldContextObject is null"));
		OnComplete.Broadcast(EEOSResult::EOS_InvalidParameters);
		SetReadyToDestroy();
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSSendReportAsync: World is null"));
		OnComplete.Broadcast(EEOSResult::EOS_InvalidParameters);
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSSendReportAsync: GameInstance is null"));
		OnComplete.Broadcast(EEOSResult::EOS_InvalidParameters);
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSSendReportAsync: EOSKitSubsystem or PlatformHandle is null"));
		OnComplete.Broadcast(EEOSResult::EOS_NotConfigured);
		SetReadyToDestroy();
		return;
	}

#if WITH_EOS_SDK
	EOS_HReports ReportsHandle = EOS_Platform_GetReportsInterface(EOSKitSubsystem->GetPlatformHandle());
	if (!ReportsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSSendReportAsync: Failed to get Reports Handle"));
		OnComplete.Broadcast(EEOSResult::EOS_NotConfigured);
		SetReadyToDestroy();
		return;
	}

	EOS_Reports_SendPlayerBehaviorReportOptions ReportOptions = {};
	ReportOptions.ApiVersion = EOS_REPORTS_SENDPLAYERBEHAVIORREPORT_API_LATEST;
	ReportOptions.ReporterUserId = ReporterUserId.GetValueAsEosType();
	ReportOptions.ReportedUserId = ReportedUserId.GetValueAsEosType();
	ReportOptions.Category = static_cast<EOS_EPlayerReportsCategory>(Category);
	
	// Truncate message if too long
	FString TruncatedMessage = Message;
	if (TruncatedMessage.Len() > EOS_REPORTS_REPORTMESSAGE_MAX_LENGTH)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSSendReportAsync: Message truncated from %d to %d characters"), 
			Message.Len(), EOS_REPORTS_REPORTMESSAGE_MAX_LENGTH);
		TruncatedMessage = TruncatedMessage.Left(EOS_REPORTS_REPORTMESSAGE_MAX_LENGTH);
	}
	ReportOptions.Message = TruncatedMessage.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*TruncatedMessage);

	// Truncate context if too long
	FString TruncatedContext = Context;
	if (TruncatedContext.Len() > EOS_REPORTS_REPORTCONTEXT_MAX_LENGTH)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSSendReportAsync: Context truncated from %d to %d characters"), 
			Context.Len(), EOS_REPORTS_REPORTCONTEXT_MAX_LENGTH);
		TruncatedContext = TruncatedContext.Left(EOS_REPORTS_REPORTCONTEXT_MAX_LENGTH);
	}
	ReportOptions.Context = TruncatedContext.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*TruncatedContext);

	EOS_Reports_SendPlayerBehaviorReport(
		ReportsHandle,
		&ReportOptions,
		this,
		(EOS_Reports_OnSendPlayerBehaviorReportCompleteCallback)&UEOSSendReportAsync::OnSendReportCompleteCallback
	);
#else
	OnComplete.Broadcast(EEOSResult::EOS_NotConfigured);
	SetReadyToDestroy();
#endif
}

#if WITH_EOS_SDK
void EOS_CALL UEOSSendReportAsync::OnSendReportCompleteCallback(const void* Data)
{
	const EOS_Reports_SendPlayerBehaviorReportCompleteCallbackInfo* CallbackInfo = reinterpret_cast<const EOS_Reports_SendPlayerBehaviorReportCompleteCallbackInfo*>(Data);

	if (!CallbackInfo || !CallbackInfo->ClientData)
	{
		return;
	}

	UEOSSendReportAsync* Self = static_cast<UEOSSendReportAsync*>(CallbackInfo->ClientData);
	EEOSResult Result = ConvertEOSResultToEEOSResult(CallbackInfo->ResultCode);

	if (Result == EEOSResult::EOS_Success)
	{
		UE_LOG(LogTemp, Log, TEXT("EOSSendReportAsync: Report sent successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSSendReportAsync: Report failed with result: %d"), static_cast<int32>(Result));
	}

	Self->OnComplete.Broadcast(Result);
	Self->SetReadyToDestroy();
}
#endif

