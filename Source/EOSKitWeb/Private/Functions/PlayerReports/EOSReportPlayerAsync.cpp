// Copyright (C) 2024, All Rights Reserved.

#include "Functions/PlayerReports/EOSReportPlayerAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_reports.h"
#include "eos_reports_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSReportPlayerAsync* UEOSReportPlayerAsync::ReportPlayer(
	UObject* WorldContextObject,
	const FString& ReporterUserId,
	const FString& ReportedUserId,
	EEOSPlayerReportCategory Category,
	const FString& Message,
	const FString& Context)
{
	UEOSReportPlayerAsync* Node = NewObject<UEOSReportPlayerAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->ReporterId = ReporterUserId;
	Node->ReportedId = ReportedUserId;
	Node->ReportCategory = Category;
	Node->ReportMessage = Message;
	Node->ReportContext = Context;
	return Node;
}

void UEOSReportPlayerAsync::Activate()
{
	PerformReport();
}

void UEOSReportPlayerAsync::PerformReport()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT("Failed to get Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT("EOSKit Subsystem not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
	EOS_HReports ReportsHandle = EOS_Platform_GetReportsInterface(PlatformHandle);

	if (!ReportsHandle)
	{
		OnFail.Broadcast(TEXT("Failed to get Reports interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert user IDs
	EOS_ProductUserId ReporterPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ReporterId));
	EOS_ProductUserId ReportedPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ReportedId));

	if (!ReporterPUID || !ReportedPUID)
	{
		OnFail.Broadcast(TEXT("Invalid Product User ID"));
		SetReadyToDestroy();
		return;
	}

	// Setup report options
	EOS_Reports_SendPlayerBehaviorReportOptions ReportOptions = {};
	ReportOptions.ApiVersion = EOS_REPORTS_SENDPLAYERBEHAVIORREPORT_API_LATEST;
	ReportOptions.ReporterUserId = ReporterPUID;
	ReportOptions.ReportedUserId = ReportedPUID;
	ReportOptions.Category = (EOS_EPlayerReportsCategory)ConvertReportCategory(ReportCategory);

	FTCHARToUTF8 MessageConverter(*ReportMessage);
	FTCHARToUTF8 ContextConverter(*ReportContext);

	if (!ReportMessage.IsEmpty())
	{
		ReportOptions.Message = MessageConverter.Get();
	}

	if (!ReportContext.IsEmpty())
	{
		ReportOptions.Context = ContextConverter.Get();
	}

	UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Sending player behavior report"));

	// Send report - use lambda wrapper
	EOS_Reports_SendPlayerBehaviorReport(ReportsHandle, &ReportOptions, this, [](const EOS_Reports_SendPlayerBehaviorReportCompleteCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSReportPlayerAsync* Self = static_cast<UEOSReportPlayerAsync*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Player report sent successfully"));
				Self->OnSuccess.Broadcast();
			}
			else
			{
				FString ErrorMessage = FString::Printf(TEXT("Report player failed: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG(LogTemp, Error, TEXT("EOSKitWeb: %s"), *ErrorMessage);
				Self->OnFail.Broadcast(ErrorMessage);
			}

			Self->SetReadyToDestroy();
		});
	});
}

uint32_t UEOSReportPlayerAsync::ConvertReportCategory(EEOSPlayerReportCategory Category)
{
	switch (Category)
	{
	case EEOSPlayerReportCategory::Cheating:
		return static_cast<uint32_t>(EOS_EPlayerReportsCategory::EOS_PRC_Cheating);
	case EEOSPlayerReportCategory::Exploiting:
		return static_cast<uint32_t>(EOS_EPlayerReportsCategory::EOS_PRC_Exploiting);
	case EEOSPlayerReportCategory::OffensiveProfile:
		return static_cast<uint32_t>(EOS_EPlayerReportsCategory::EOS_PRC_OffensiveProfile);
	case EEOSPlayerReportCategory::VerbalAbuse:
		return static_cast<uint32_t>(EOS_EPlayerReportsCategory::EOS_PRC_VerbalAbuse);
	case EEOSPlayerReportCategory::Scamming:
		return static_cast<uint32_t>(EOS_EPlayerReportsCategory::EOS_PRC_Scamming);
	case EEOSPlayerReportCategory::Spamming:
		return static_cast<uint32_t>(EOS_EPlayerReportsCategory::EOS_PRC_Spamming);
	case EEOSPlayerReportCategory::Other:
		return static_cast<uint32_t>(EOS_EPlayerReportsCategory::EOS_PRC_Other);
	default:
		return static_cast<uint32_t>(EOS_EPlayerReportsCategory::EOS_PRC_Invalid);
	}
}
