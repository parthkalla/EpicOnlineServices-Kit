// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Sessions_QueryInvites.h"

UEOK_Sessions_QueryInvites* UEOK_Sessions_QueryInvites::EOK_Sessions_QueryInvites(FEOK_ProductUserId LocalUserId)
{
	UEOK_Sessions_QueryInvites* Node = NewObject<UEOK_Sessions_QueryInvites>();
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOK_Sessions_QueryInvites::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sessions_QueryInvitesOptions QueryInvitesOptions = { };
			QueryInvitesOptions.ApiVersion = EOS_SESSIONS_QUERYINVITES_API_LATEST;
			QueryInvitesOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_Sessions_QueryInvites(EOSRef->SessionsHandle, &QueryInvitesOptions, this, &UEOK_Sessions_QueryInvites::OnQueryInvitesCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query invites either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Sessions_QueryInvites::OnQueryInvitesCallback(const EOS_Sessions_QueryInvitesCallbackInfo* Data)
{
	if (UEOK_Sessions_QueryInvites* Node = static_cast<UEOK_Sessions_QueryInvites*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
