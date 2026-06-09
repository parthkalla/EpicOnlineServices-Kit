// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Auth_Login.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Auth_Login* UEOK_Auth_Login::Login(FEOK_Auth_LoginOptions Options)
{
	UEOK_Auth_Login* Node = NewObject<UEOK_Auth_Login>();
	Node->Var_Options = Options;
	return Node;
}

void UEOK_Auth_Login::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Auth_LoginOptions Options = Var_Options.ToEOSAuthLoginOptions();
			EOS_Auth_Login(EOSRef->AuthHandle, &Options, this, &UEOK_Auth_Login::Internal_OnLoginComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to login either OnlineSubsystem is not valid or EOSRef is not valid."));
	FEOK_Auth_LoginCallbackInfo Info;
	Info.Result = EEOK_Result::EOS_NotFound;
	OnCallback.Broadcast(Info);
}

void UEOK_Auth_Login::Internal_OnLoginComplete(const EOS_Auth_LoginCallbackInfo* Data)
{
	UEOK_Auth_Login* Node = static_cast<UEOK_Auth_Login*>(Data->ClientData);
	if(Node)
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(*Data);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}
