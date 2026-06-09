// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sessions_StartSession.h"

#include "Async/Async.h"

UEOK_Sessions_StartSession* UEOK_Sessions_StartSession::EOK_Sessions_StartSession(FString SessionName)
{
	UEOK_Sessions_StartSession* Node = NewObject<UEOK_Sessions_StartSession>();
	Node->Var_SessionName = SessionName;
	return Node;
}

void UEOK_Sessions_StartSession::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sessions_StartSessionOptions StartSessionOptions = { };
			StartSessionOptions.ApiVersion = EOS_SESSIONS_STARTSESSION_API_LATEST;
			StartSessionOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
			EOS_Sessions_StartSession(EOSRef->SessionsHandle, &StartSessionOptions, this, &UEOK_Sessions_StartSession::OnStartSessionCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to start session either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sessions_StartSession::OnStartSessionCallback(const EOS_Sessions_StartSessionCallbackInfo* Data)
{
	if (UEOK_Sessions_StartSession* Node = static_cast<UEOK_Sessions_StartSession*>(Data->ClientData))
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
