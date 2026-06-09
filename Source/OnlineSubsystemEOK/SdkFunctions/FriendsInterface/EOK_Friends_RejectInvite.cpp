// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Friends_RejectInvite.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Friends_RejectInvite* UEOK_Friends_RejectInvite::EOK_Friends_RejectInvite(FEOK_EpicAccountId LocalUserId,
	FEOK_EpicAccountId TargetUserId)
{
	UEOK_Friends_RejectInvite* Node = NewObject<UEOK_Friends_RejectInvite>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Friends_RejectInvite::OnRejectInviteCallback(const EOS_Friends_RejectInviteCallbackInfo* Data)
{
	if (UEOK_Friends_RejectInvite* Node = static_cast<UEOK_Friends_RejectInvite*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, Data->TargetUserId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Friends_RejectInvite::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_RejectInviteOptions RejectInviteOptions = {};
			RejectInviteOptions.ApiVersion = EOS_FRIENDS_REJECTINVITE_API_LATEST;
			RejectInviteOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			RejectInviteOptions.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			EOS_Friends_RejectInvite(EOSRef->FriendsHandle, &RejectInviteOptions, this, &UEOK_Friends_RejectInvite::OnRejectInviteCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to reject friend invite either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_EpicAccountId(), FEOK_EpicAccountId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
