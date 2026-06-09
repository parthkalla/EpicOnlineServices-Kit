// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Connect_Login.h"

#include "EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"

UEOK_Connect_Login* UEOK_Connect_Login::EOK_Connect_Login(FEOK_Connect_Credentials Credentials,
                                                          FEOK_Connect_UserLoginInfo UserLoginInfo)
{
	UEOK_Connect_Login* Node = NewObject<UEOK_Connect_Login>();
	Node->Local_Credentials = Credentials;
	Node->Local_UserLoginInfo = UserLoginInfo;
	return Node;
}

void UEOK_Connect_Login::OnLoginCallback(const EOS_Connect_LoginCallbackInfo* Data)
{
	if(UEOK_Connect_Login* Node = static_cast<UEOK_Connect_Login*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, Data->ContinuanceToken);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Connect_Login::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_LoginOptions LoginOptions = { };
			LoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
			EOS_Connect_Credentials Cred = Local_Credentials.ToEOSConnectCredentials();
			EOS_Connect_UserLoginInfo UserLoginInfo = Local_UserLoginInfo.ToEOSConnectUserLoginInfo();
			LoginOptions.Credentials = &Cred;
			LoginOptions.UserLoginInfo = &UserLoginInfo;
			EOS_Connect_Login(EOSRef->ConnectHandle, &LoginOptions, this, &UEOK_Connect_Login::OnLoginCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to login user either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_ProductUserId(), FEOK_ContinuanceToken());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
