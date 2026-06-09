// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_DestroyLobby.h"

#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Lobby_DestroyLobby* UEOK_Lobby_DestroyLobby::EOK_Lobby_DestroyLobby(FEOK_ProductUserId LocalUserId,
	FEOK_LobbyId LobbyId)
{
	UEOK_Lobby_DestroyLobby* LobbyDestroyLobby = NewObject<UEOK_Lobby_DestroyLobby>();
	LobbyDestroyLobby->Var_LocalUserId = LocalUserId;
	LobbyDestroyLobby->Var_LobbyId = LobbyId;
	return LobbyDestroyLobby;
}

void UEOK_Lobby_DestroyLobby::OnDestroyLobbyComplete(const EOS_Lobby_DestroyLobbyCallbackInfo* Data)
{
	if (UEOK_Lobby_DestroyLobby* Node = static_cast<UEOK_Lobby_DestroyLobby*>(Data->ClientData))
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

void UEOK_Lobby_DestroyLobby::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_DestroyLobbyOptions Options = {};
			Options.ApiVersion = EOS_LOBBY_DESTROYLOBBY_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.LobbyId = Var_LobbyId.GetValueAsEosType();
			EOS_Lobby_DestroyLobby(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_DestroyLobby::OnDestroyLobbyComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to destroy lobby either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
