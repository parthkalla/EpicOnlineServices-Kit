// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Auth_QueryIdToken.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Auth_QueryIdToken* UEOK_Auth_QueryIdToken::QueryIdToken(FEOK_EpicAccountId LocalUserId,
	FEOK_EpicAccountId TargetAccountId)
{
	UEOK_Auth_QueryIdToken* Node = NewObject<UEOK_Auth_QueryIdToken>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetAccountId = TargetAccountId;
	return Node;
}

void UEOK_Auth_QueryIdToken::OnQueryIdTokenCallback(const EOS_Auth_QueryIdTokenCallbackInfo* Data)
{
	UEOK_Auth_QueryIdToken* Node = static_cast<UEOK_Auth_QueryIdToken*>(Data->ClientData);
	if(Node)
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, Data->TargetAccountId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Auth_QueryIdToken::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_QueryIdTokenOptions Options = {};
			Options.ApiVersion = EOS_AUTH_QUERYIDTOKEN_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.TargetAccountId = Var_TargetAccountId.GetValueAsEosType();
			EOS_Auth_QueryIdToken(EOSRef->AuthHandle, &Options, this, &UEOK_Auth_QueryIdToken::OnQueryIdTokenCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to query id token either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, Var_LocalUserId, Var_TargetAccountId);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
