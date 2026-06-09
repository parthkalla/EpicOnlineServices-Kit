// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Friends_SendInvite.h"

UEOK_Friends_SendInvite* UEOK_Friends_SendInvite::EOK_Friends_SendInvite(FEOK_EpicAccountId LocalUserId,
	FEOK_EpicAccountId TargetUserId)
{
	UEOK_Friends_SendInvite* Node = NewObject<UEOK_Friends_SendInvite>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Friends_SendInvite::OnSendInviteCallback(const EOS_Friends_SendInviteCallbackInfo* Data)
{
	if (UEOK_Friends_SendInvite* Node = static_cast<UEOK_Friends_SendInvite*>(Data->ClientData))
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

void UEOK_Friends_SendInvite::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_SendInviteOptions SendInviteOptions = {};
			SendInviteOptions.ApiVersion = EOS_FRIENDS_SENDINVITE_API_LATEST;
			SendInviteOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			SendInviteOptions.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			EOS_Friends_SendInvite(EOSRef->FriendsHandle, &SendInviteOptions, this, &UEOK_Friends_SendInvite::OnSendInviteCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to send friend invite either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_EpicAccountId(), FEOK_EpicAccountId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
