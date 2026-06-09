// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sessions_SendInvite.h"

#include "Async/Async.h"

UEOK_Sessions_SendInvite* UEOK_Sessions_SendInvite::EOK_Sessions_SendInvite(FString SessionName,
                                                                            FEOK_ProductUserId LocalUserId, FEOK_ProductUserId TargetUserId)
{
	UEOK_Sessions_SendInvite* Node = NewObject<UEOK_Sessions_SendInvite>();
	Node->Var_SessionName = SessionName;
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Sessions_SendInvite::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sessions_SendInviteOptions SendInviteOptions = { };
			SendInviteOptions.ApiVersion = EOS_SESSIONS_SENDINVITE_API_LATEST;
			SendInviteOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
			SendInviteOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			SendInviteOptions.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			EOS_Sessions_SendInvite(EOSRef->SessionsHandle, &SendInviteOptions, this, &UEOK_Sessions_SendInvite::OnSendInviteCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to send invite either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sessions_SendInvite::OnSendInviteCallback(const EOS_Sessions_SendInviteCallbackInfo* Data)
{
	if (UEOK_Sessions_SendInvite* Node = static_cast<UEOK_Sessions_SendInvite*>(Data->ClientData))
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
