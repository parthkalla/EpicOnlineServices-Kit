// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Connect_UnlinkAccount.h"

#include "EOK_ConnectSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"

UEOK_Connect_UnlinkAccount* UEOK_Connect_UnlinkAccount::EOK_Connect_UnlinkAccount(FEOK_ProductUserId LocalUserId)
{
	UEOK_Connect_UnlinkAccount* Node = NewObject<UEOK_Connect_UnlinkAccount>();
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOK_Connect_UnlinkAccount::OnUnlinkAccountCallback(const EOS_Connect_UnlinkAccountCallbackInfo* Data)
{
	if(UEOK_Connect_UnlinkAccount* Proxy = static_cast<UEOK_Connect_UnlinkAccount*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Proxy, Data]()
		{
			Proxy->OnCallback.Broadcast(FEOK_ProductUserId(Data->LocalUserId), static_cast<EEOK_Result>(Data->ResultCode));
		});
		Proxy->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Proxy->MarkAsGarbage();
#else
		Proxy->MarkPendingKill();
#endif
	}
}

void UEOK_Connect_UnlinkAccount::Activate()
{
	Super::Activate();
	if(IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if(FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Connect_UnlinkAccountOptions UnlinkAccountOptions = {};
			UnlinkAccountOptions.ApiVersion = EOS_CONNECT_UNLINKACCOUNT_API_LATEST;
			UnlinkAccountOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			EOS_Connect_UnlinkAccount(EOSRef->ConnectHandle, &UnlinkAccountOptions, this, &UEOK_Connect_UnlinkAccount::OnUnlinkAccountCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to unlink account either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_ProductUserId(), EEOK_Result::EOS_ServiceFailure);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}


