// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_JoinLobby.h"

#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Lobby_JoinLobby* UEOK_Lobby_JoinLobby::EOK_Lobby_JoinLobby(FEOK_Lobby_JoinLobbyOptions Options)
{
	UEOK_Lobby_JoinLobby* Node = NewObject<UEOK_Lobby_JoinLobby>();
	Node->Var_Options = Options;
	return Node;
}

void UEOK_Lobby_JoinLobby::OnJoinLobbyComplete(const EOS_Lobby_JoinLobbyCallbackInfo* Data)
{
	if (UEOK_Lobby_JoinLobby* Node = static_cast<UEOK_Lobby_JoinLobby*>(Data->ClientData))
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

void UEOK_Lobby_JoinLobby::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_JoinLobbyOptions Options = Var_Options.ToEOSOptions();
			EOS_Lobby_JoinLobby(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_JoinLobby::OnJoinLobbyComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to join lobby either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
