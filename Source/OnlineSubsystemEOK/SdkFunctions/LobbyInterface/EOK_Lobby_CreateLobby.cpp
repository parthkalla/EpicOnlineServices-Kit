// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_CreateLobby.h"

#include "OnlineSessionEOS.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Lobby_CreateLobby* UEOK_Lobby_CreateLobby::EOK_Lobby_CreateLobby(FEOK_Lobby_CreateLobbyOptions CreateLobbyOptions)
{
	UEOK_Lobby_CreateLobby* LobbyCreateLobby = NewObject<UEOK_Lobby_CreateLobby>();
	LobbyCreateLobby->Var_CreateLobbyOptions = CreateLobbyOptions;
	return LobbyCreateLobby;
}

void UEOK_Lobby_CreateLobby::OnCreateLobbyComplete(const EOS_Lobby_CreateLobbyCallbackInfo* Data)
{
	if (UEOK_Lobby_CreateLobby* Node = static_cast<UEOK_Lobby_CreateLobby*>(Data->ClientData))
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
void UEOK_Lobby_CreateLobby::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_CreateLobbyOptions Options = Var_CreateLobbyOptions.GetCreateLobbyOptions();
			EOS_Lobby_CreateLobby(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_CreateLobby::OnCreateLobbyComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to create lobby either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
