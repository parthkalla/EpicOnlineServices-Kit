// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Friends_AcceptInvite.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Friends_AcceptInvite* UEOK_Friends_AcceptInvite::EOK_Friends_AcceptInvite(FEOK_EpicAccountId LocalUserId,
                                                                               FEOK_EpicAccountId TargetUserId)
{
	UEOK_Friends_AcceptInvite* Node = NewObject<UEOK_Friends_AcceptInvite>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Friends_AcceptInvite::OnAcceptInviteCallback(const EOS_Friends_AcceptInviteCallbackInfo* Data)
{
	if (UEOK_Friends_AcceptInvite* Node = static_cast<UEOK_Friends_AcceptInvite*>(Data->ClientData))
	{
		Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, Data->TargetUserId);
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Friends_AcceptInvite::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_AcceptInviteOptions AcceptInviteOptions = {};
			AcceptInviteOptions.ApiVersion = EOS_FRIENDS_ACCEPTINVITE_API_LATEST;
			AcceptInviteOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			AcceptInviteOptions.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			EOS_Friends_AcceptInvite(EOSRef->FriendsHandle, &AcceptInviteOptions, this, &UEOK_Friends_AcceptInvite::OnAcceptInviteCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to accept friend invite either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_EpicAccountId(), FEOK_EpicAccountId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
