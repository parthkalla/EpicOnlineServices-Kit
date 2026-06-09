// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_RejectInvite.h"

UEOK_Lobby_RejectInvite* UEOK_Lobby_RejectInvite::EOK_Lobby_RejectInvite(FEOK_ProductUserId LocalUserId,
	FString InviteId)
{
	UEOK_Lobby_RejectInvite* Node = NewObject<UEOK_Lobby_RejectInvite>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_InviteId = InviteId;
	return Node;
}

void UEOK_Lobby_RejectInvite::OnRejectInviteComplete(const EOS_Lobby_RejectInviteCallbackInfo* Data)
{
	if (UEOK_Lobby_RejectInvite* Node = static_cast<UEOK_Lobby_RejectInvite*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), FString());
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}

void UEOK_Lobby_RejectInvite::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_RejectInviteOptions Options = { };
			Options.ApiVersion = EOS_LOBBY_REJECTINVITE_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.InviteId = TCHAR_TO_ANSI(*Var_InviteId);
			EOS_Lobby_RejectInvite(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_RejectInvite::OnRejectInviteComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to reject invite either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FString());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
