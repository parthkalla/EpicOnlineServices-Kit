// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_JoinLobbyById.h"

#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Lobby_JoinLobbyById* UEOK_Lobby_JoinLobbyById::EOK_Lobby_JoinLobbyById(FEOK_Lobby_JoinLobbyByIdOptions Options)
{
	UEOK_Lobby_JoinLobbyById* Node = NewObject<UEOK_Lobby_JoinLobbyById>();
	Node->Var_Options = Options;
	return Node;
}

void UEOK_Lobby_JoinLobbyById::OnJoinLobbyByIdComplete(const EOS_Lobby_JoinLobbyByIdCallbackInfo* Data)
{
	if (UEOK_Lobby_JoinLobbyById* Node = static_cast<UEOK_Lobby_JoinLobbyById*>(Data->ClientData))
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

void UEOK_Lobby_JoinLobbyById::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_JoinLobbyByIdOptions Options = Var_Options.ToEOSOptions();
			EOS_Lobby_JoinLobbyById(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_JoinLobbyById::OnJoinLobbyByIdComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to join lobby by id either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
