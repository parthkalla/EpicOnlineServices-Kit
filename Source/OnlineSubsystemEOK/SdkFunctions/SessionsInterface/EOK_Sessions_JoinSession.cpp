// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sessions_JoinSession.h"

#include "Async/Async.h"

UEOK_Sessions_JoinSession* UEOK_Sessions_JoinSession::EOK_Sessions_JoinSession(FString SessionName,
                                                                               FEOK_ProductUserId LocalUserId, FEOK_HSessionDetails SessionHandle, bool bPresenceEnabled)
{
	UEOK_Sessions_JoinSession* Node = NewObject<UEOK_Sessions_JoinSession>();
	Node->Var_SessionName = SessionName;
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_SessionHandle = SessionHandle;
	Node->Var_bPresenceEnabled = bPresenceEnabled;
	return Node;
}

void UEOK_Sessions_JoinSession::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sessions_JoinSessionOptions JoinSessionOptions = { };
			JoinSessionOptions.ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
			JoinSessionOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
			JoinSessionOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			JoinSessionOptions.SessionHandle = Var_SessionHandle.Ref;
			JoinSessionOptions.bPresenceEnabled = Var_bPresenceEnabled;
			EOS_Sessions_JoinSession(EOSRef->SessionsHandle, &JoinSessionOptions, this, &UEOK_Sessions_JoinSession::OnJoinSessionCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to join session either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sessions_JoinSession::OnJoinSessionCallback(const EOS_Sessions_JoinSessionCallbackInfo* Data)
{
	if (UEOK_Sessions_JoinSession* Node = static_cast<UEOK_Sessions_JoinSession*>(Data->ClientData))
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
