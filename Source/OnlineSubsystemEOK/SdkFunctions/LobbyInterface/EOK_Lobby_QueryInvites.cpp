// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_QueryInvites.h"

UEOK_Lobby_QueryInvites* UEOK_Lobby_QueryInvites::EOK_Lobby_QueryInvites(FEOK_ProductUserId LocalUserId)
{
	UEOK_Lobby_QueryInvites* Node = NewObject<UEOK_Lobby_QueryInvites>();
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOK_Lobby_QueryInvites::OnQueryInvitesComplete(const EOS_Lobby_QueryInvitesCallbackInfo* Data)
{
	if (UEOK_Lobby_QueryInvites* Node = static_cast<UEOK_Lobby_QueryInvites*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), FEOK_LobbyId());
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}

void UEOK_Lobby_QueryInvites::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_QueryInvitesOptions Options = { };
			Options.ApiVersion = EOS_LOBBY_QUERYINVITES_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_Lobby_QueryInvites(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_QueryInvites::OnQueryInvitesComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query invites either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
