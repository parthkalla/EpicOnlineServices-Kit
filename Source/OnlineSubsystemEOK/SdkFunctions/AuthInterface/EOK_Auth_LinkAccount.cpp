// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Auth_LinkAccount.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Auth_LinkAccount* UEOK_Auth_LinkAccount::LinkAccount(FEOK_ContinuanceToken ContinuanceToken,
	FEOK_EpicAccountId LocalUserId, TEnumAsByte<EEOK_ELinkAccountFlags> LinkAccountFlags)
{
	UEOK_Auth_LinkAccount* Node = NewObject<UEOK_Auth_LinkAccount>();
	Node->Var_ContinuanceToken = ContinuanceToken;
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_LinkAccountFlags = LinkAccountFlags;
	return Node;
}

void UEOK_Auth_LinkAccount::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_LinkAccountOptions Options = {};
			Options.ApiVersion = EOS_AUTH_LINKACCOUNT_API_LATEST;
			Options.ContinuanceToken = Var_ContinuanceToken.GetValueAsEosType();
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.LinkAccountFlags = static_cast<EOS_ELinkAccountFlags>(Var_LinkAccountFlags.GetValue());
			EOS_Auth_LinkAccount(EOSRef->AuthHandle, &Options, this, &UEOK_Auth_LinkAccount::OnLinkAccountCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to link account either OnlineSubsystem is not valid or EOSRef is not valid."));
	FEOK_Auth_LinkAccountCallbackInfo Info;
	Info.ResultCode = EEOK_Result::EOS_NotFound;
	OnCallback.Broadcast(Info);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Auth_LinkAccount::OnLinkAccountCallback(const EOS_Auth_LinkAccountCallbackInfo* Data)
{
	UEOK_Auth_LinkAccount* Node = static_cast<UEOK_Auth_LinkAccount*>(Data->ClientData);
	if(Node)
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(Data);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
