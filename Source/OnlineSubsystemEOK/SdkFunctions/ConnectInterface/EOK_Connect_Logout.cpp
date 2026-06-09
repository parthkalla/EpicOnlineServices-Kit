// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Connect_Logout.h"

#include "EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"

UEOK_Connect_Logout* UEOK_Connect_Logout::EOK_Connect_Logout(FEOK_ProductUserId ProductUserId)
{
	UEOK_Connect_Logout* Proxy = NewObject<UEOK_Connect_Logout>();
	Proxy->Var_ProductUserId = ProductUserId;
	return Proxy;
}

void UEOK_Connect_Logout::OnLogoutCallback(const EOS_Connect_LogoutCallbackInfo* Data)
{
	if(UEOK_Connect_Logout* Proxy = static_cast<UEOK_Connect_Logout*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Proxy, Data]()
		{
			Proxy->OnCallback.Broadcast(Proxy->Var_ProductUserId);
		});
		Proxy->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Proxy->MarkAsGarbage();
#else
		Proxy->MarkPendingKill();
#endif
		}
}

void UEOK_Connect_Logout::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_LogoutOptions LogoutOptions = { };
			LogoutOptions.ApiVersion = EOS_CONNECT_LOGOUT_API_LATEST;
			LogoutOptions.LocalUserId = Var_ProductUserId.GetValueAsEosType();
			EOS_Connect_Logout(EOSRef->ConnectHandle, &LogoutOptions, this, &UEOK_Connect_Logout::OnLogoutCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to logout user either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_ProductUserId());
};
