// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_LeaveLobby.h"

UEOK_Lobby_LeaveLobby* UEOK_Lobby_LeaveLobby::EOK_Lobby_LeaveLobby(FEOK_ProductUserId LocalUserId, FEOK_LobbyId LobbyId)
{
	UEOK_Lobby_LeaveLobby* Node = NewObject<UEOK_Lobby_LeaveLobby>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_LobbyId = LobbyId;
	return Node;
}

void UEOK_Lobby_LeaveLobby::OnLeaveLobbyComplete(const EOS_Lobby_LeaveLobbyCallbackInfo* Data)
{
	if (UEOK_Lobby_LeaveLobby* Node = static_cast<UEOK_Lobby_LeaveLobby*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LobbyId);
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}

void UEOK_Lobby_LeaveLobby::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_LeaveLobbyOptions Options = { };
			Options.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
			Options.LobbyId = Var_LobbyId.GetValueAsEosType();
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_Lobby_LeaveLobby(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_LeaveLobby::OnLeaveLobbyComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to leave lobby either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
