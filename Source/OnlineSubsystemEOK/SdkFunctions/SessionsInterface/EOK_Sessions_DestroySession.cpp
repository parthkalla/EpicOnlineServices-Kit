// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sessions_DestroySession.h"

#include "Async/Async.h"

UEOK_Sessions_DestroySession* UEOK_Sessions_DestroySession::EOK_Sessions_DestroySession(FString SessionName)
{
	UEOK_Sessions_DestroySession* Node = NewObject<UEOK_Sessions_DestroySession>();
	Node->Var_SessionName = SessionName;
	return Node;
}

void UEOK_Sessions_DestroySession::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sessions_DestroySessionOptions DestroySessionOptions = { };
			DestroySessionOptions.ApiVersion = EOS_SESSIONS_DESTROYSESSION_API_LATEST;
			DestroySessionOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
			EOS_Sessions_DestroySession(EOSRef->SessionsHandle, &DestroySessionOptions, this, &UEOK_Sessions_DestroySession::OnDestroySessionCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to destroy session either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sessions_DestroySession::OnDestroySessionCallback(const EOS_Sessions_DestroySessionCallbackInfo* Data)
{
	if(UEOK_Sessions_DestroySession* Node = static_cast<UEOK_Sessions_DestroySession*>(Data->ClientData))
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
