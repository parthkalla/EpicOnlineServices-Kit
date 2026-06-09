// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sessions_UpdateSession.h"

#include "Async/Async.h"

UEOK_Sessions_UpdateSession* UEOK_Sessions_UpdateSession::EOK_Sessions_UpdateSession(
	FEOK_HSessionModification SessionModificationHandle)
{
	UEOK_Sessions_UpdateSession* Node = NewObject<UEOK_Sessions_UpdateSession>();
	Node->Var_SessionModificationHandle = SessionModificationHandle;
	return Node;
}

void UEOK_Sessions_UpdateSession::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sessions_UpdateSessionOptions UpdateSessionOptions = { };
			UpdateSessionOptions.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
			UpdateSessionOptions.SessionModificationHandle = Var_SessionModificationHandle.Ref;
			EOS_Sessions_UpdateSession(EOSRef->SessionsHandle, &UpdateSessionOptions, this, &UEOK_Sessions_UpdateSession::OnUpdateSessionCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to update session either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, "", "");
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sessions_UpdateSession::OnUpdateSessionCallback(const EOS_Sessions_UpdateSessionCallbackInfo* Data)
{
	if (UEOK_Sessions_UpdateSession* Node = static_cast<UEOK_Sessions_UpdateSession*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->SessionName, Data->SessionId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
