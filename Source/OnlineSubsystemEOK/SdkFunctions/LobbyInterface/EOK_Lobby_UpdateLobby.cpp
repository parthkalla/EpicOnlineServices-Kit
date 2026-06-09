// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_UpdateLobby.h"

UEOK_Lobby_UpdateLobby* UEOK_Lobby_UpdateLobby::EOK_Lobby_UpdateLobby(FEOK_HLobbyModification LobbyModificationHandle)
{
	UEOK_Lobby_UpdateLobby* Node = NewObject<UEOK_Lobby_UpdateLobby>();
	Node->Var_LobbyModificationHandle = LobbyModificationHandle;
	return Node;
}

void UEOK_Lobby_UpdateLobby::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_UpdateLobbyOptions Options = { };
			Options.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
			Options.LobbyModificationHandle = *Var_LobbyModificationHandle.Ref;
			EOS_Lobby_UpdateLobby(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_UpdateLobby::OnUpdateLobbyComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to update lobby either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
void UEOK_Lobby_UpdateLobby::OnUpdateLobbyComplete(const EOS_Lobby_UpdateLobbyCallbackInfo* Data)
{
	if (UEOK_Lobby_UpdateLobby* Node = static_cast<UEOK_Lobby_UpdateLobby*>(Data->ClientData))
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