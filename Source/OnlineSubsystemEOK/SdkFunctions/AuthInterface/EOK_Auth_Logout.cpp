// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Auth_Logout.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Auth_Logout* UEOK_Auth_Logout::Logout(FEOK_EpicAccountId EpicAccountId)
{
	UEOK_Auth_Logout* Node = NewObject<UEOK_Auth_Logout>();
	Node->Var_EpicAccountId = EpicAccountId;
	return Node;
}

void UEOK_Auth_Logout::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_LogoutOptions Options = {};
			Options.ApiVersion = EOS_AUTH_LOGOUT_API_LATEST;
			Options.LocalUserId = Var_EpicAccountId.GetValueAsEosType();
			EOS_Auth_Logout(EOSRef->AuthHandle, &Options, this, &UEOK_Auth_Logout::OnLogoutCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to logout either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, Var_EpicAccountId);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Auth_Logout::OnLogoutCallback(const EOS_Auth_LogoutCallbackInfo* Data)
{
	UEOK_Auth_Logout* Node = static_cast<UEOK_Auth_Logout*>(Data->ClientData);
	if(Node)
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
