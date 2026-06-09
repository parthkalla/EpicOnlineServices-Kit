// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sessions_EndSession.h"

#include "Async/Async.h"

UEOK_Sessions_EndSession* UEOK_Sessions_EndSession::EOK_Sessions_EndSession(FString SessionName)
{
	UEOK_Sessions_EndSession* BlueprintNode = NewObject<UEOK_Sessions_EndSession>();
	BlueprintNode->Var_SessionName = SessionName;
	return BlueprintNode;
}

void UEOK_Sessions_EndSession::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sessions_EndSessionOptions EndSessionOptions = { };
			EndSessionOptions.ApiVersion = EOS_SESSIONS_ENDSESSION_API_LATEST;
			EndSessionOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
			EOS_Sessions_EndSession(EOSRef->SessionsHandle, &EndSessionOptions, this, &UEOK_Sessions_EndSession::OnEndSessionCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to end session either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sessions_EndSession::OnEndSessionCallback(const EOS_Sessions_EndSessionCallbackInfo* Data)
{
	if (UEOK_Sessions_EndSession* Node = static_cast<UEOK_Sessions_EndSession*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode));
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
