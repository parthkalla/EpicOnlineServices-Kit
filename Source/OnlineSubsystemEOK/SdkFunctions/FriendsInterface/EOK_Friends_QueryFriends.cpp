// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Friends_QueryFriends.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Friends_QueryFriends* UEOK_Friends_QueryFriends::EOK_Friends_QueryFriends(FEOK_EpicAccountId LocalUserId)
{
	UEOK_Friends_QueryFriends* Node = NewObject<UEOK_Friends_QueryFriends>();
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOK_Friends_QueryFriends::OnQueryFriendsCallback(const EOS_Friends_QueryFriendsCallbackInfo* Data)
{
	if (UEOK_Friends_QueryFriends* Node = static_cast<UEOK_Friends_QueryFriends*>(Data->ClientData))
	{
		Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId);
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Friends_QueryFriends::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Friends_QueryFriendsOptions QueryFriendsOptions = {};
			QueryFriendsOptions.ApiVersion = EOS_FRIENDS_QUERYFRIENDS_API_LATEST;
			QueryFriendsOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_Friends_QueryFriends(EOSRef->FriendsHandle, &QueryFriendsOptions, this, &UEOK_Friends_QueryFriends::OnQueryFriendsCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query friends either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_EpicAccountId());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
