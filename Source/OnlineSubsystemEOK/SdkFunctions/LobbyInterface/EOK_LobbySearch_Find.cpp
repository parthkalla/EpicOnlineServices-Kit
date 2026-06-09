// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_LobbySearch_Find.h"


UEOK_LobbySearch_Find* UEOK_LobbySearch_Find::EOK_LobbySearch_Find(FEOK_HLobbySearch Handle,
	FEOK_ProductUserId LocalUserId)
{
	UEOK_LobbySearch_Find* Node = NewObject<UEOK_LobbySearch_Find>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_Handle = Handle;
	return Node;
}

void UEOK_LobbySearch_Find::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_LobbySearch_FindOptions Options = { };
			Options.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_LobbySearch_Find(Var_Handle.Ref, &Options, this, &UEOK_LobbySearch_Find::OnFindComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to find lobby either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_LobbySearch_Find::OnFindComplete(const EOS_LobbySearch_FindCallbackInfo* Data)
{
	if (UEOK_LobbySearch_Find* Node = static_cast<UEOK_LobbySearch_Find*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode));
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
Node->MarkAsGarbage();
#else
Node->MarkPendingKill();
#endif
		});
	}
}
