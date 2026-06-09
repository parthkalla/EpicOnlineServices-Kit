// Copyright Epic Games, Inc. All Rights Reserved.

#include "EOK_OpenReportsUI_AsyncFunction.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"

UEOK_OpenReportsUI_AsyncFunction* UEOK_OpenReportsUI_AsyncFunction::ShowEOKPlayerReportUIAsyncFunction(FString LocalReporterPUID, FString TargetPlayerPUID)
{
	UEOK_OpenReportsUI_AsyncFunction* UEOK_ShowReportUI_Object = NewObject < UEOK_OpenReportsUI_AsyncFunction>();
	UEOK_ShowReportUI_Object->LocalReporterPUID = LocalReporterPUID;
	UEOK_ShowReportUI_Object->TargetPlayerPUID = TargetPlayerPUID;
	return UEOK_ShowReportUI_Object;
}

void UEOK_OpenReportsUI_AsyncFunction::SendReportFunc()
{
    if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
    {
        if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
        {
            if (EOSRef->ReportsHandle != nullptr)
            {
                if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
                {
                    EOS_UI_ShowReportPlayerOptions ShowReportPlayerOptions;
                    ShowReportPlayerOptions.ApiVersion = EOS_UI_SHOWREPORTPLAYER_API_LATEST;
                    ShowReportPlayerOptions.LocalUserId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*LocalReporterPUID));
                    ShowReportPlayerOptions.TargetUserId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*TargetPlayerPUID));

                    EOS_UI_ShowReportPlayer(EOSRef->UIHandle, &ShowReportPlayerOptions, this, SendReportFuncCallback);
                }
                else
                {
                    ResultFaliure();
                }
            }
            else
            {
                ResultFaliure();
            }
        }
        else
        {
            ResultFaliure();
        }
    }
    else
    {
        ResultFaliure();
    }
}

void UEOK_OpenReportsUI_AsyncFunction::SendReportFuncCallback(const EOS_UI_OnShowReportPlayerCallbackInfo* Data)
{
    if (UEOK_OpenReportsUI_AsyncFunction* ShowReportUIFunction = static_cast<UEOK_OpenReportsUI_AsyncFunction*>(Data->ClientData))
    {
        if (Data->ResultCode == EOS_EResult::EOS_Success)
        {
            ShowReportUIFunction->ResultSuccess();
        }
        else
        {
            ShowReportUIFunction->ResultFaliure();
        }
    }
}

void UEOK_OpenReportsUI_AsyncFunction::ResultFaliure()
{
    Failure.Broadcast();
    SetReadyToDestroy();
}

void UEOK_OpenReportsUI_AsyncFunction::ResultSuccess()
{
    Success.Broadcast();
    SetReadyToDestroy();
}

void UEOK_OpenReportsUI_AsyncFunction::Activate()
{
    SendReportFunc();
    Super::Activate();
}
