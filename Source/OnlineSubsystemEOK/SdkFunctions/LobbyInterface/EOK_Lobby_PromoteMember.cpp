// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_PromoteMember.h"

UEOK_Lobby_PromoteMember* UEOK_Lobby_PromoteMember::EOK_Lobby_PromoteMember(FEOK_LobbyId LobbyId,
	FEOK_ProductUserId ProductUserId, FEOK_ProductUserId TargetUserId)
{
	UEOK_Lobby_PromoteMember* Node = NewObject<UEOK_Lobby_PromoteMember>();
	Node->Var_LobbyId = LobbyId;
	Node->Var_ProductUserId = ProductUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Lobby_PromoteMember::OnPromoteMemberComplete(const EOS_Lobby_PromoteMemberCallbackInfo* Data)
{
	if (UEOK_Lobby_PromoteMember* Node = static_cast<UEOK_Lobby_PromoteMember*>(Data->ClientData))
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

void UEOK_Lobby_PromoteMember::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Lobby_PromoteMemberOptions Options = { };
			Options.ApiVersion = EOS_LOBBY_PROMOTEMEMBER_API_LATEST;
			Options.LobbyId = Var_LobbyId.GetValueAsEosType();
			Options.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			Options.LocalUserId = Var_ProductUserId.GetValueAsEosType();
			EOS_Lobby_PromoteMember(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_PromoteMember::OnPromoteMemberComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to promote member either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
